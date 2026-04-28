import { posix } from 'node:path'

const { join } = posix
import { patchesDir, worktreeDir } from './config.js'
import {
  cd,
  cloneUpstream,
  ensureStgInstalled,
  ensureUpstreamRemote,
  getAllPatchNames,
  getAppliedPatchNames,
  hasGitRepo,
  hasLocalBranch,
  hasStgitStack,
  linkForkSource,
  patchNameFromSeriesEntry,
  readPinnedUpstreamCommit,
  readSeries,
  step,
  success,
  updateSubmodules,
} from './lib.js'

const BRANCH = 'arcanegram'

function sameOrder(actual: string[], expected: string[]) {
  return actual.length === expected.length && actual.every((v, i) => v === expected[i])
}

async function ensureWorktree(commit: string) {
  if (!hasGitRepo(worktreeDir)) {
    await cloneUpstream(worktreeDir, commit)
    return
  }
  step(`reusing existing checkout in ${worktreeDir}`)
  await ensureUpstreamRemote(worktreeDir)
  await updateSubmodules(worktreeDir)
}

async function ensureBranch(commit: string) {
  const repo = cd(worktreeDir)
  const existed = await hasLocalBranch(worktreeDir, BRANCH)
  if (existed) {
    step(`checking out ${BRANCH}`)
    await repo`git checkout ${BRANCH}`
  }
  else {
    step(`creating local work branch at ${commit}`)
    await repo`git checkout -B ${BRANCH} ${commit}`
  }
  return existed
}

async function ensureStgitStack(commit: string, branchExisted: boolean) {
  const repo = cd(worktreeDir)
  await repo`git config stgit.namelength 120`
  await repo`git config commit.gpgsign false`
  await repo`git config tag.gpgsign false`

  if (hasStgitStack(worktreeDir, BRANCH)) {
    step('stgit stack already initialized')
    return
  }

  if (branchExisted) {
    const head = (await repo`git rev-parse HEAD`).stdout.trim()
    if (head !== commit) {
      throw new Error(`branch ${BRANCH} exists without stgit metadata and is not at ${commit}`)
    }
  }

  step('initializing stgit')
  await repo`stg init`
}

async function importSeries(seriesEntries: string[]) {
  const repo = cd(worktreeDir)
  for (const entry of seriesEntries) {
    const patchName = patchNameFromSeriesEntry(entry)
    step(`importing ${entry} as ${patchName}`)
    await repo`stg import -n ${patchName} ${join(patchesDir, entry)}`
  }
}

async function ensurePatches(expected: string[], seriesEntries: string[]) {
  const repo = cd(worktreeDir)
  const existing = await getAllPatchNames(worktreeDir)

  if (existing.length === 0) {
    await importSeries(seriesEntries)
  }
  else if (!sameOrder(existing, expected)) {
    throw new Error('existing stgit stack does not match series (use --force to reset)')
  }

  const applied = await getAppliedPatchNames(worktreeDir)
  if (!sameOrder(applied, expected)) {
    step('pushing remaining patches')
    await repo`stg push -a`
  }
}

async function forceReimportPatches(seriesEntries: string[]) {
  const repo = cd(worktreeDir)
  const dirty = (await repo`git status --porcelain`).stdout.trim()
  if (dirty) throw new Error(`refusing to force-reimport: worktree is dirty: ${dirty}`)

  const existing = await getAllPatchNames(worktreeDir)
  const applied = await getAppliedPatchNames(worktreeDir)
  if (applied.length !== existing.length) {
    throw new Error(`refusing to force-reimport: ${existing.length - applied.length} patch(es) above top not applied`)
  }

  if (existing.length > 0) {
    step(`popping ${existing.length} patch(es)`)
    await repo`stg pop -a`
    for (const name of existing) {
      step(`deleting ${name}`)
      await repo`stg delete ${name}`
    }
  }
  await importSeries(seriesEntries)
}

const args = process.argv.slice(2)
const force = args.includes('--force')
const noStgit = args.includes('--no-stgit')

await ensureStgInstalled()
const commit = await readPinnedUpstreamCommit()
const seriesEntries = await readSeries()

if (noStgit) {
  if (hasGitRepo(worktreeDir)) {
    throw new Error(`--no-stgit needs a fresh worktree (${worktreeDir} already exists)`)
  }
  await cloneUpstream(worktreeDir, commit)
  const repo = cd(worktreeDir)
  for (const entry of seriesEntries) {
    step(`applying ${entry}`)
    await repo`git apply ${join(patchesDir, entry)}`
  }
  await linkForkSource(worktreeDir)
  success('flat setup complete')
}
else {
  const expectedPatches = seriesEntries.map(patchNameFromSeriesEntry)
  await ensureWorktree(commit)
  const branchExisted = await ensureBranch(commit)
  await ensureStgitStack(commit, branchExisted)
  if (force) await forceReimportPatches(seriesEntries)
  else await ensurePatches(expectedPatches, seriesEntries)
  const linkedAny = await linkForkSource(worktreeDir)
  success(linkedAny ? 'setup complete' : 'up to date')
}
