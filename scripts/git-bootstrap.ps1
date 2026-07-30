# Creates an initial, thematic commit history for the repo (Windows/PowerShell).
# Run once from the repository root after installing git:
#   ./scripts/git-bootstrap.ps1
# Uses your existing git identity (user.name / user.email). Adds no AI attribution.
$ErrorActionPreference = "Stop"

if (Test-Path ".git") {
  Write-Error "This is already a git repository. Aborting."
}
if (-not (Test-Path "README.md")) {
  Write-Error "Run this from the repository root (README.md not found)."
}

# Verify identity is configured so commits are attributed to you.
$name = (git config user.name)
$email = (git config user.email)
if (-not $name -or -not $email) {
  Write-Error "Set your git identity first: git config --global user.name '...'; git config --global user.email '...'"
}

git init -b main | Out-Null

# Ordered, thematic commits. Each stages a coherent slice of the project.
$commits = @(
  @{ msg = "chore: project scaffolding, license, and editor config";
     paths = @(".gitignore", ".gitattributes", ".editorconfig", ".dockerignore", ".env.example", "LICENSE") },
  @{ msg = "feat(gateway): C++ API gateway with JWT/API-key auth, RBAC, rate limiting, routing, metrics";
     paths = @("gateway") },
  @{ msg = "feat(auth-service): Python service issuing signed JWTs with role-to-permission expansion";
     paths = @("services/auth-service/app", "services/auth-service/requirements.txt", "services/auth-service/Dockerfile") },
  @{ msg = "feat(orders-service): C# orders microservice with read/write RBAC";
     paths = @("services/orders-service") },
  @{ msg = "feat(catalog-service): Ruby catalog microservice reachable by JWT or API key";
     paths = @("services/catalog-service") },
  @{ msg = "feat(dashboard): React + TypeScript admin console";
     paths = @("dashboard") },
  @{ msg = "build: Docker Compose orchestration";
     paths = @("docker-compose.yml") },
  @{ msg = "build(deploy): Kubernetes manifests and Helm chart";
     paths = @("deploy") },
  @{ msg = "ci: GitHub Actions, CodeQL, issue/PR templates, dependabot";
     paths = @(".github") },
  @{ msg = "feat(observability): Prometheus and Grafana provisioning";
     paths = @("monitoring", "docker-compose.observability.yml") },
  @{ msg = "test: gateway crypto/JWT unit tests and auth-service tests";
     paths = @("gateway/tests", "services/auth-service/tests") },
  @{ msg = "docs: architecture, configuration, RBAC, deployment, and OpenAPI spec";
     paths = @("docs", "CONTRIBUTING.md", "CODE_OF_CONDUCT.md", "SECURITY.md", "CHANGELOG.md") },
  @{ msg = "chore: developer scripts and Makefile";
     paths = @("scripts", "Makefile") },
  @{ msg = "docs: project README";
     paths = @("README.md") }
)

foreach ($c in $commits) {
  foreach ($p in $c.paths) {
    if (Test-Path $p) { git add -- $p }
  }
  # Only commit if something was actually staged.
  git diff --cached --quiet
  if ($LASTEXITCODE -ne 0) {
    git commit -q -m $c.msg
    Write-Host "committed: $($c.msg)"
  }
}

# Catch anything not explicitly grouped above.
git add -A
git diff --cached --quiet
if ($LASTEXITCODE -ne 0) {
  git commit -q -m "chore: remaining project files"
  Write-Host "committed: chore: remaining project files"
}

Write-Host ""
Write-Host "Done. $(git rev-list --count HEAD) commits on 'main'."
Write-Host "Next: create the repo on GitHub, then:"
Write-Host "  git remote add origin git@github.com:<you>/apiway.git"
Write-Host "  git push -u origin main"
