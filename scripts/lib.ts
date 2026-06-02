import { existsSync } from "node:fs";
import fs from "node:fs/promises";
import {
  basename,
  dirname,
  isAbsolute,
  posix,
  relative,
  resolve,
} from "node:path";

// use posix join so paths stay forward-slash on windows (zx shell escaping mangles `\a`/`\b`/etc).
const { join } = posix;
import { glob } from "tinyglobby";
import { $, chalk, which } from "zx";
import {
  forkSyncFiles,
  rootDir,
  seriesFile,
  upstreamCommitFile,
  upstreamUrl,
} from "./config.js";

$.verbose = false;
// On Windows, zx may pick up bash (Git Bash / WSL) where native binaries
// like stg.exe aren't on PATH. Force cmd.exe so all spawns use the same
// environment that the user's shell has.
if (process.platform === "win32") {
  $.shell = "cmd.exe";
  $.shellFlags = "/d /s /c";
  $.prefix = ""; // zx prepends `set -euo pipefail;` by default; cmd.exe treats it as an env-var name
}

export function step(message: string) {
  console.log(`${chalk.blue("==>")} ${message}`);
}
export function success(message: string) {
  console.log(`${chalk.green("ok")} ${message}`);
}
export function warn(message: string) {
  console.log(`${chalk.yellow("warn")} ${message}`);
}

export function cd(cwd: string) {
  return $({ cwd });
}

export function resolveFromRoot(input: string | undefined, fallback?: string) {
  if (!input) {
    if (fallback) return fallback;
    throw new Error("missing required path argument");
  }
  return isAbsolute(input) ? input : resolve(rootDir, input);
}

export async function readPinnedUpstreamCommit() {
  const value = (await fs.readFile(upstreamCommitFile, "utf8")).trim();
  if (!/^[0-9a-f]{7,40}$/i.test(value)) {
    throw new Error(
      `set ${relative(rootDir, upstreamCommitFile)} to a real commit hash first`,
    );
  }
  return value;
}

export async function writePinnedUpstreamCommit(commit: string) {
  await fs.writeFile(upstreamCommitFile, `${commit}\n`);
}

export async function ensureDir(dir: string) {
  await fs.mkdir(dir, { recursive: true });
}

export async function ensureEmptyCloneTarget(targetDir: string) {
  if (!existsSync(targetDir)) return;
  const entries = await fs.readdir(targetDir);
  if (entries.length > 0 && !entries.includes(".git")) {
    throw new Error(`target exists and is not empty: ${targetDir}`);
  }
}

export async function cloneUpstream(targetDir: string, commit: string) {
  await ensureEmptyCloneTarget(targetDir);
  if (!existsSync(join(targetDir, ".git"))) {
    step(`cloning upstream into ${targetDir}`);
    await $`git clone --recurse-submodules ${upstreamUrl} ${targetDir}`;
  } else {
    step(`reusing existing checkout in ${targetDir}`);
  }
  await ensureUpstreamRemote(targetDir);
  const git = cd(targetDir);
  step(`checking out ${commit}`);
  await git`git checkout ${commit}`;
  step("updating submodules");
  await git`git submodule update --init --recursive`;
}

export async function ensureUpstreamRemote(repoDir: string) {
  const git = cd(repoDir);
  const remotes = (await git`git remote`).stdout
    .split(/\r?\n/)
    .map((s) => s.trim())
    .filter(Boolean);
  if (remotes.includes("upstream")) return;
  if (remotes.includes("origin")) {
    step("renaming origin remote to upstream");
    await git`git remote rename origin upstream`;
    return;
  }
  step("adding upstream remote");
  await git`git remote add upstream ${upstreamUrl}`;
}

export function hasGitRepo(repoDir: string) {
  return existsSync(join(repoDir, ".git"));
}

export function hasStgitStack(repoDir: string, branch: string) {
  return existsSync(join(repoDir, ".git", "refs", "stacks", branch));
}

export async function hasLocalBranch(repoDir: string, branch: string) {
  const result = await $({
    cwd: repoDir,
    nothrow: true,
  })`git show-ref --verify --quiet refs/heads/${branch}`;
  return result.exitCode === 0;
}

async function isTrackedPath(repoDir: string, repoRelativePath: string) {
  const result = await $({
    cwd: repoDir,
    nothrow: true,
  })`git ls-files --error-unmatch -- ${repoRelativePath}`;
  return result.exitCode === 0;
}

async function ensureSkipWorktree(repoDir: string, repoRelativePath: string) {
  if (!(await isTrackedPath(repoDir, repoRelativePath))) return false;
  const status = (
    await $({ cwd: repoDir })`git ls-files -v -- ${repoRelativePath}`
  ).stdout.trim();
  if (status.startsWith("S ")) return true;
  step(`marking ${repoRelativePath} as skip-worktree`);
  await $({
    cwd: repoDir,
  })`git update-index --skip-worktree -- ${repoRelativePath}`;
  return true;
}

async function ensureSymlinkOrCopy(
  targetPath: string,
  srcPath: string,
  type: "dir" | "file",
) {
  await ensureDir(dirname(targetPath));
  const stat = await fs.lstat(targetPath).catch(() => null);

  if (type === "dir") {
    if (stat?.isSymbolicLink()) {
      const currentTarget = resolve(
        dirname(targetPath),
        await fs.readlink(targetPath),
      );
      if (currentTarget === srcPath) return false;
    }
    if (stat) await fs.rm(targetPath, { recursive: true, force: true });
    // 'junction' on windows avoids needing admin/dev-mode; on posix node ignores type and creates a regular symlink.
    await fs.symlink(srcPath, targetPath, "junction");
    return true;
  }

  if (stat) {
    if (stat.isSymbolicLink()) {
      const currentTarget = resolve(
        dirname(targetPath),
        await fs.readlink(targetPath),
      );
      if (currentTarget === srcPath) return false;
    }
    await fs.rm(targetPath, { force: true });
  }
  try {
    await fs.symlink(
      relative(dirname(targetPath), srcPath),
      targetPath,
      "file",
    );
  } catch {
    await fs.copyFile(srcPath, targetPath);
  }
  return true;
}

interface ResolvedLink {
  sourcePath: string;
  repoRelativeTarget: string;
  type: "dir" | "file";
  replace?: boolean;
}

async function linkForkEntry(repoDir: string, entry: ResolvedLink) {
  const targetPath = join(repoDir, entry.repoRelativeTarget);
  const created = await ensureSymlinkOrCopy(
    targetPath,
    entry.sourcePath,
    entry.type,
  );
  if (created) step(`linked ${targetPath}`);

  if (
    entry.replace &&
    (await ensureSkipWorktree(repoDir, entry.repoRelativeTarget))
  ) {
    return created;
  }
  await ensureGitExclude(repoDir, entry.repoRelativeTarget);
  return created;
}

export async function linkForkSource(repoDir: string) {
  let dirty = false;
  for (const entry of forkSyncFiles) {
    if (entry.directory) {
      const created = await linkForkEntry(repoDir, {
        sourcePath: resolve(rootDir, entry.source),
        repoRelativeTarget: entry.target,
        type: "dir",
        replace: entry.replace,
      });
      dirty ||= created;
      continue;
    }
    const matches = await glob(entry.source, {
      cwd: rootDir,
      absolute: true,
      onlyFiles: true,
    });
    for (const sourcePath of matches) {
      const created = await linkForkEntry(repoDir, {
        sourcePath,
        repoRelativeTarget: join(entry.target, basename(sourcePath)),
        type: "file",
        replace: entry.replace,
      });
      dirty ||= created;
    }
  }
  return dirty;
}

export async function ensureGitExclude(
  repoDir: string,
  repoRelativePath: string,
) {
  const excludeFile = join(repoDir, ".git", "info", "exclude");
  const entry = repoRelativePath.replaceAll("\\", "/");
  const current = await fs.readFile(excludeFile, "utf8").catch(() => "");
  const lines = current.split(/\r?\n/);
  if (lines.includes(entry)) return;
  step(`adding ${entry} to .git/info/exclude`);
  const next =
    current.length === 0 || current.endsWith("\n")
      ? `${current}${entry}\n`
      : `${current}\n${entry}\n`;
  await fs.writeFile(excludeFile, next);
}

function normalizeSeriesLine(line: string) {
  const trimmed = line.trim();
  if (!trimmed) return "";
  return trimmed.replace(/^[+>!-]\s+/, "");
}

async function getPatchNames(repoDir: string, mode: "--applied" | "--all") {
  const stg = cd(repoDir);
  const out = await stg`stg series ${mode}`;
  return out.stdout
    .split(/\r?\n/)
    .map(normalizeSeriesLine)
    .map((s) => s.trim())
    .filter(Boolean);
}

export async function getAppliedPatchNames(repoDir: string) {
  return getPatchNames(repoDir, "--applied");
}

export async function getAllPatchNames(repoDir: string) {
  return getPatchNames(repoDir, "--all");
}

export function patchNameFromSeriesEntry(entry: string) {
  const normalized = entry.trim().replaceAll("\\", "/");
  const match = normalized.match(/^([^/]+)\/(.+)\.patch$/);
  if (!match) throw new Error(`invalid series entry: ${entry}`);
  return `${match[1]}__${match[2]}`;
}

export async function getPatchCommitId(repoDir: string, patchName: string) {
  return (await cd(repoDir)`stg id ${patchName}`).stdout.trim();
}

export async function writeSeries(entries: string[]) {
  await fs.writeFile(
    seriesFile,
    entries.length > 0 ? `${entries.join("\n")}\n` : "",
  );
  step(
    `wrote ${entries.length} ${entries.length === 1 ? "entry" : "entries"} to series`,
  );
}

export async function readSeries() {
  const raw = await fs.readFile(seriesFile, "utf8").catch(() => "");
  return raw
    .split(/\r?\n/)
    .map((s) => s.trim())
    .filter(Boolean);
}

export async function updateSubmodules(repoDir: string) {
  step("updating submodules");
  await cd(repoDir)`git submodule update --init --recursive`;
}

export async function ensureStgInstalled() {
  const path = await which("stg", { nothrow: true });
  if (!path) {
    throw new Error(
      "stgit (`stg`) not found on PATH. install via Git for Windows / MSYS2 / your package manager.",
    );
  }
}
