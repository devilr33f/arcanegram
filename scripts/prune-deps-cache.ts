import { existsSync } from 'node:fs'
import { rm } from 'node:fs/promises'
import { join } from 'node:path'
import { glob } from 'tinyglobby'
import { rootDir } from './config.js'

const libsRoot = join(rootDir, 'worktree/Libraries/win64').replaceAll('\\', '/')

// remove patterns: paths to delete relative to libsRoot.
// add subprojects here once Step 1 confirms they don't break a downstream cmake configure.
const removePatterns: string[] = [
  // openssl: keep out32/, drop everything else
  'openssl_3/!(out32|include)',
  // qt: keep out/ install dir, drop the source/build tree
  'qt_*/qtbase/build*',
  'qt_*/qtbase/.obj*',
  'qt_*/!(out|qtbase|qtimageformats|qtsvg|qtwayland|qtshadertools)',
  // ffmpeg: keep .a/.lib outputs and headers
  'ffmpeg/lib*/!(*.lib|*.a)',
  // tg_owt: keep out/Release, drop src obj trees
  'tg_owt/out/!(Release)',
  'tg_owt/src',
  'tg_owt/third_party',
  // libavif: keep final products
  'libavif/build/CMakeFiles',
  'libavif/build/_deps',
  // openal-soft: keep build/Release
  'openal-soft/build/!(Release)',
  // generic intermediate files anywhere
  '**/*.obj',
  '**/*.pdb',
  '**/CMakeFiles',
]

async function prune() {
  if (!existsSync(libsRoot)) {
    console.log(`[prune] ${libsRoot} does not exist, nothing to do`)
    return
  }

  for (const pattern of removePatterns) {
    const matches = await glob(pattern, { cwd: libsRoot, dot: true, onlyFiles: false, expandDirectories: false })
    for (const rel of matches) {
      const abs = join(libsRoot, rel)
      try {
        await rm(abs, { recursive: true, force: true })
        console.log(`[prune] removed ${rel}`)
      }
      catch (err) {
        console.warn(`[prune] failed ${rel}: ${(err as Error).message}`)
      }
    }
  }
  console.log(`[prune] done`)
}

await prune()
