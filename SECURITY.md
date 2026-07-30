# Security Policy

## Supported versions

| Version | Supported |
|---------|-----------|
| 1.x     | ✅        |
| < 1.0   | ❌        |

## Reporting a vulnerability

Please **do not** open a public issue for security vulnerabilities.

Instead, use GitHub's [private vulnerability reporting][gh-report] on this
repository, or email the maintainers. We aim to acknowledge reports within 72
hours and to ship a fix or mitigation within 30 days for confirmed issues.

When reporting, please include:

- A description of the vulnerability and its impact
- Steps to reproduce (a proof of concept is ideal)
- Affected component(s): `gateway`, `auth-service`, `orders-service`,
  `catalog-service`, or `dashboard`

## Security model notes

- The gateway is the trust boundary. Downstream services trust the `X-Auth-*`
  headers it sets and must not be exposed directly to clients.
- JWTs are signed with HS256 using a shared secret. Rotate `JWT_SECRET`
  periodically and keep it out of version control (`.env` is gitignored).
- API keys are bearer credentials. Treat them like passwords.

[gh-report]: https://docs.github.com/en/code-security/security-advisories/guidance-on-reporting-and-writing-information-about-vulnerabilities/privately-reporting-a-security-vulnerability
