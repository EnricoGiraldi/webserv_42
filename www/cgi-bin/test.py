#!/usr/bin/env python3

# Import CGI module
import cgi

# Print the HTTP header
print("Content-Type: text/html")
print("")  # Blank line to separate headers from the body

# Print the HTML content
print("<html>")
print("<head><title>Test CGI</title></head>")
print("<body>")
print("<h1>CGI Test</h1>")
print("<p>This is a response from the Python CGI script.</p>")
print("</body>")
print("</html>")
