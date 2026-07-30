# Contributing to Apiway

Thanks for your interest in improving Apiway! This project is polyglot by
design, so contributions can land in any of C++, Python, C#, Ruby, or the
TypeScript dashboard.

## Ways to contribute

- **Report bugs** — open an issue with reproduction steps.
- **Propose features** — open a discussion or a feature-request issue first.
- **Improve docs** — everything under `docs/` is fair game.
- **Send code** — see the workflow below.

## Development setup

The fastest path is Docker:

```bash
cp .env.example .env      # set a strong JWT_SECRET
docker compose up --build
```

To work on a single component, see [`docs/getting-started.md`](docs/getting-started.md).

## Pull request workflow

1. Fork and create a topic branch: `git checkout -b feat/short-description`.
2. Keep changes focused; one logical change per PR.
3. Match the style of the surrounding code. Formatting is enforced in CI:
   - C++ — `clang-format` (LLVM style, 4-space indent)
   - Python — `ruff` + `black`
   - TypeScript — `eslint` + `prettier`
4. Add or update tests. Run `make test` locally.
5. Update `CHANGELOG.md` under the `Unreleased` heading.
6. Open the PR and fill in the template. CI must be green before review.

## Commit messages

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat(gateway): add per-route circuit breaker
fix(auth): reject tokens with a future nbf claim
docs: clarify RBAC role mapping
```

## Code of conduct

Participation is governed by our [Code of Conduct](CODE_OF_CONDUCT.md).
