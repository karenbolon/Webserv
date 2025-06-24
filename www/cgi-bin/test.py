#!/usr/bin/env python3
import os
import cgi

print("Content-Type: text/html\r\n\r\n")
print("<html><head><title>CGI Environment Debug</title></head><body>")
print("<h1>CGI Environment Variables</h1>")

# All environment variables
print("<h2>All Environment Variables:</h2>")
print("<ul>")
for key in sorted(os.environ.keys()):
    value = os.environ[key]
    print(f"<li><strong>{key}</strong>: {value}</li>")
print("</ul>")

# HTTP headers specifically
print("<h2>HTTP Headers (HTTP_* variables):</h2>")
print("<ul>")
http_vars = {k: v for k, v in os.environ.items() if k.startswith('HTTP_')}
if http_vars:
    for key in sorted(http_vars.keys()):
        print(f"<li><strong>{key}</strong>: {http_vars[key]}</li>")
else:
    print("<li><em>No HTTP_* variables found!</em></li>")
print("</ul>")

# Check for specific important headers
print("<h2>Important Headers Check:</h2>")
important_headers = ['HTTP_COOKIE', 'HTTP_HOST', 'HTTP_USER_AGENT', 'HTTP_ACCEPT']
for header in important_headers:
    value = os.environ.get(header, "NOT FOUND")
    print(f"<p><strong>{header}</strong>: {value}</p>")

print("</body></html>")