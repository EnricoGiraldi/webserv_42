#!/usr/bin/env python3

# Importa il modulo cgi (se necessario per eventuali elaborazioni dei dati inviati dal client)
import cgi

# Stampa l'header HTTP: specifica il Content-Type e una riga vuota per separare gli header dal corpo.
print("Content-Type: text/html")
print("")  # Linea vuota obbligatoria

# Stampa il contenuto HTML della risposta
print("<html>")
print("<head><title>Test CGI</title></head>")
print("<body>")
print("<h1>CGI Test</h1>")
print("<p>This is a response from the Python CGI script.</p>")
print("</body>")
print("</html>")
