#!/usr/bin/perl

print "Content-type: text/html; charset=utf-8\r\n\r\n";

print <<"END";
<html>
<head>
<title>CGI with perl</title>
<style>
  body {
    background-image: url('https://upload.wikimedia.org/wikipedia/commons/9/97/The_Earth_seen_from_Apollo_17.jpg');
    background-size: cover;
    color: black;
    font-family: Arial, sans-serif;
    text-align: center;
    padding-top: 100px;
  }
</style>
</head>
<body>
    <h1>CGI with perl</h1>
    <h2>Hello World!</h2>
    <div style="margin-top: 20px;">
      <form method="GET" action="/form.html">
         <button type="submit">🔙 Back to Forms</button>
      </form>
    </div>
</body>
</html>
END