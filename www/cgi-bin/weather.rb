#!/usr/bin/env ruby

require 'net/http'
require 'uri'

puts "Content-Type: text/html; charset=utf-8\r\n\r\n"
puts

uri = URI("https://wttr.in/Berlin?format=3")

weather = Net::HTTP.get(uri)

puts '<html><head><style>body {
            background-color: #fcd8f7;
            font-family: Arial, sans-serif;
            padding-top: 100px;
        }
    </style>
    </head><body>'
puts "<h1>CGI: Ruby Ruby Ruby!!</h1>"
puts "<h2>Today's Weather</h2>"
puts "<p>#{weather}</p>"
puts ' <div style="margin-top: 20px;">
        <form method="GET" action="/form.html">
           <button type="submit">🔙 Back to Forms</button>
        </form>
    </div>'
puts "</body></html>"