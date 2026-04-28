import { $ } from 'zx'
import { worktreeDir } from './config.js'
import {
  cd,
  getAllPatchNames,
  step,
  success,
  updateSubmodules,
  warn,
  writePinnedUpstreamCommit,
} from './lib.js'

const target = process.argv[2]
if (!target) throw new Error("new upstream commit or 'latest' required")

const repo = cd(worktreeDir)
step('fetching upstream')
await repo`git fetch upstream`

const resolvedCommit = target === 'latest'
  ? (await repo`git rev-parse upstream/HEAD`).stdout.trim()
  : (await repo`git rev-parse ${target}^{commit}`).stdout.trim()

await writePinnedUpstreamCommit(resolvedCommit)

step(`rebasing stack onto ${resolvedCommit}`)
const result = await $({ cwd: worktreeDir, nothrow: true })`stg rebase ${resolvedCommit}`

if (result.exitCode !== 0) {
  const patchNames = await getAllPatchNames(worktreeDir).catch(() => [])
  const conflicts = patchNames.filter(p => result.stdout.includes(p) || result.stderr.includes(p))
  if (conflicts.length > 0) warn(`conflicts in: ${conflicts.join(', ')}`)
  else warn('rebase reported conflicts')
  if (result.stderr.trim()) console.error(result.stderr.trim())
  process.exit(result.exitCode ?? 1)
}

await updateSubmodules(worktreeDir)
success(`rebased onto ${resolvedCommit}`)
