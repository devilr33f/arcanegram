import { dirname, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

// normalize to forward slashes so zx-shell-escaped paths don't mangle backslash sequences on windows.
const posix = (p: string) => p.replaceAll('\\', '/')

export const upstreamUrl = 'https://github.com/telegramdesktop/tdesktop'
export const rootDir = posix(resolve(dirname(fileURLToPath(import.meta.url)), '..'))
export const worktreeDir = posix(join(rootDir, 'worktree'))
export const patchesDir = posix(join(rootDir, 'patches'))
export const seriesFile = posix(join(rootDir, 'series'))
export const upstreamCommitFile = posix(join(rootDir, 'upstream-commit'))

export interface ForkSyncFile {
  source: string
  target: string
  directory?: boolean
  replace?: boolean
}

export const forkSyncFiles: ForkSyncFile[] = [
  // arcanegram.style lives inside src/cpp/arcanegram/ so it ships via the directory junction.
  { source: 'src/cpp/arcanegram', target: 'Telegram/SourceFiles/arcanegram', directory: true },
  { source: 'res/lang/lang_arcanegram.strings', target: 'Telegram/Resources/langs' },
  { source: 'res/icons/settings/*', target: 'Telegram/Resources/icons/arcanegram/settings' },
]
