# 🛡️ Security Policy

## Scope of This Repository
`windows-dll-loading-rnd` is a defensive systems-engineering and Windows Internals research project. All binaries compiled by this repository are non-malicious educational harnesses.

## Reporting Security or Build Issues
If you discover a bug in the PE32+ parser (`src/pe_report/pe_report.c`), such as an out-of-bounds read on malformed section headers, or an issue in the build pipeline:
1. Open a GitHub Issue describing the reproduction steps and input header values.
2. Provide a minimal patch or pull request preserving `-Wall -Wextra -Werror -Wpedantic` compliance.
