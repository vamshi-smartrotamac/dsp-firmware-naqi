# CI/CD setup — self-hosted org pattern

`dsp-firmware-naqi` uses the org CI pattern, but the shared infrastructure is temporarily hosted in a
**personal** repo (`vamshi-smartrotamac/max78000-ci-workflows`) so all checks pass without org-admin.

## One-time bring-up (all doable by the repo owner)
1. **Create & push `max78000-ci-workflows` (public):**
   ```bash
   cd /c/Users/user/OneDrive/Documents/GitHub/max78000-ci-workflows
   git init -b main && git add . && git commit -m "MSDK CI: image + reusable workflows/actions"
   gh repo create vamshi-smartrotamac/max78000-ci-workflows --public --source=. --push
   ```
2. **Publish the toolchain image:** the push to `main` triggers `max78000-docker-push.yml`
   (or run it via Actions → "Publish docker image" → Run workflow). It publishes
   `ghcr.io/vamshi-smartrotamac/max78000-toolchain:v2024_10`.
3. **Make the package public:** GitHub → your packages → `max78000-toolchain` → Package settings →
   Change visibility → Public. (Lets `dsp-firmware-naqi` pull it with no extra access config.)
4. **Push `dsp-firmware-naqi`** and open the PR — `build` pulls the image + ccache action,
   `clang Check` runs the reusable check (code is pre-formatted → passes), `preprocess` labels the PR.
5. **Release:** push a semver tag `X.Y.Z` → `deploy.yml` builds and publishes the release with
   `.elf/.bin/.hex` + a changelog from `Feature`/`Fix` PR labels.

## Migration to Naqi-Logix (later, needs org access)
Prefer PR-ing the MSDK image + `max78000-docker-push.yml` into the existing `NaqiLogix/ci-workflows`
(it already has the clang/label workflows + ccache/release actions). Then in this repo, rename:
- `ghcr.io/vamshi-smartrotamac/max78000-toolchain` → `ghcr.io/naqilogix/max78000-toolchain`
- `vamshi-smartrotamac/max78000-ci-workflows@main` → `NaqiLogix/ci-workflows@master`
and transfer `dsp-firmware-naqi` into the org.

## Already done on the repo side
- Org-pattern workflows (pull central image, reuse actions, semver release, ccache, PR labeling).
- Source formatted to team `.clang-format` (clang-format 15) → blocking clang check passes.
- Case-sensitivity + FreeRTOS stack-overflow-hook fixes → debug + release compile.
