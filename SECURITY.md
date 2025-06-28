# Security Policy

## Reporting Security Vulnerabilities

If you discover a security vulnerability in dir2txt, please report it by emailing the project maintainers. Please do not create public issues for security vulnerabilities.

## Security Features

dir2txt includes several security features to protect against common vulnerabilities:

### Path Traversal Protection
- Symlinks pointing outside the target directory are automatically blocked
- Directory paths are canonicalized to prevent traversal attacks
- Comprehensive input validation on all paths

### Resource Protection
- Regex pattern validation to prevent ReDoS (Regular Expression Denial of Service)
- File size limits for ignore files (1MB max)
- Pattern count limits (10,000 patterns max per file)
- Pattern length limits (1000 characters max)

### Error Handling
- Graceful handling of permission denied errors
- Robust exception handling for filesystem operations
- Secure error messages that don't leak sensitive information

## Security Best Practices

When using dir2txt:

1. **Run with minimal privileges**: Don't run as root unless necessary
2. **Validate input directories**: Ensure you trust the source directory
3. **Review ignore patterns**: Be cautious with complex regex patterns
4. **Monitor output**: Check output files are written to expected locations

## Security Audit

dir2txt has undergone a comprehensive security audit. Key findings:

- ✅ No critical vulnerabilities in current version
- ✅ Symlink following vulnerability fixed
- ✅ ReDoS vulnerability mitigated  
- ✅ Exception handling hardened
- ✅ Input validation improved

For detailed audit results, see the security audit report.

## Supported Versions

Security updates are provided for:
- Latest main branch
- Current release tags

## Security Contact

For security-related questions or to report vulnerabilities:
- Email: [Contact project maintainers]
- Response time: 48-72 hours for critical issues