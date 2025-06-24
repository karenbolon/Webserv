#!/usr/bin/env ruby

require 'net/http'
require 'uri'

puts "Content-Type: text/html; charset=utf-8\r\n\r\n"

# Get weather with error handling
begin
	uri = URI("https://wttr.in/Berlin?format=3")
	weather = Net::HTTP.get(uri).strip
  rescue => e
	weather = "🌤️ Berlin 15°C (Weather service temporarily unavailable)"
  end

  # Determine theme based on weather
  if weather.downcase.include?('sun') || weather.downcase.include?('clear')
	bg_color = 'linear-gradient(135deg, #FFD700, #FFA500, #FF6347)'
	primary_color = '#FF6347'
	icon = '☀️'
	particles = '🌞'
  elsif weather.downcase.include?('rain') || weather.downcase.include?('drizzle')
	bg_color = 'linear-gradient(135deg, #4682B4, #1E90FF, #00CED1)'
	primary_color = '#1E90FF'
	icon = '🌧️'
	particles = '💧'
  elsif weather.downcase.include?('snow')
	bg_color = 'linear-gradient(135deg, #E6E6FA, #B0C4DE, #87CEEB)'
	primary_color = '#4169E1'
	icon = '❄️'
	particles = '❄️'
  elsif weather.downcase.include?('cloud')
	bg_color = 'linear-gradient(135deg, #708090, #C0C0C0, #D3D3D3)'
	primary_color = '#696969'
	icon = '☁️'
	particles = '☁️'
  else
	bg_color = 'linear-gradient(135deg, #87CEEB, #98FB98, #F0E68C)'
	primary_color = '#4682B4'
	icon = '🌤️'
	particles = '✨'
  end

puts '<html>'
puts '<head>'
puts '<style>'
puts 'body {'
puts '  font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;'
puts "  background: #{bg_color};"
puts '  background-size: 400% 400%;'
puts '  animation: gradient-shift 10s ease-in-out infinite;'
puts '  min-height: 100vh;'
puts '  margin: 0;'
puts '  display: flex;'
puts '  align-items: center;'
puts '  justify-content: center;'
puts '  padding: 20px;'
puts '  position: relative;'
puts '  overflow: hidden;'
puts '}'

puts '.weather-particle {'
puts '  position: absolute;'
puts '  font-size: 1.5rem;'
puts '  opacity: 0.6;'
puts '  animation: float-down 8s linear infinite;'
puts '  pointer-events: none;'
puts '}'

puts '.weather-box {'
puts '  max-width: 600px;'
puts '  width: 100%;'
puts '  background: rgba(255, 255, 255, 0.95);'
puts '  border-radius: 25px;'
puts '  padding: 40px;'
puts '  text-align: center;'
puts '  box-shadow: 0 20px 40px rgba(0, 0, 0, 0.2);'
puts '  backdrop-filter: blur(10px);'
puts '  position: relative;'
puts '  z-index: 10;'
puts '}'

puts '@keyframes gradient-shift {'
puts '  0%, 100% { background-position: 0% 50%; }'
puts '  50% { background-position: 100% 50%; }'
puts '}'

puts '@keyframes float-down {'
puts '  0% { transform: translateY(-100px) rotate(0deg); }'
puts '  100% { transform: translateY(100vh) rotate(360deg); }'
puts '}'

puts '.weather-icon {'
puts '  font-size: 5rem;'
puts '  margin: 20px 0;'
puts '  animation: bounce 2s ease-in-out infinite;'
puts "  text-shadow: 0 0 20px #{primary_color};"
puts '}'

puts '@keyframes bounce {'
puts '  0%, 100% { transform: translateY(0); }'
puts '  50% { transform: translateY(-15px); }'
puts '}'

puts '.weather-text {'
puts '  font-size: 1.5rem;'
puts '  color: #333;'
puts '  margin: 20px 0;'
puts '}'
puts '.btn {'
puts '  background: #4682B4;'
puts '  color: white;'
puts '  padding: 12px 24px;'
puts '  border: none;'
puts '  border-radius: 15px;'
puts '  font-size: 1rem;'
puts '  margin: 10px;'
puts '  text-decoration: none;'
puts '  display: inline-block;'
puts '}'
puts '.btn:hover {'
puts '  background: #5a9bd4;'
puts '}'
puts '</style>'
puts '</head>'
puts '<body>'

# Add floating particles
4.times do |i|
	left = 10 + (i * 25)
	delay = i * 2
	puts "<div class=\"weather-particle\" style=\"left: #{left}%; animation-delay: #{delay}s;\">#{particles}</div>"
end

puts '<div class="weather-box">'
puts "<div class=\"weather-icon\">#{icon}</div>"
puts '<h1>🌤️ Weather Station</h1>'
puts '<div class="weather-text">' + weather + '</div>'
puts '<p>Current conditions in Berlin, Germany</p>'
puts '<br>'
puts '<a href="/cgi-bin/weather.rb" class="btn">🔄 Refresh</a>'
puts '<a href="/web/playground.html" class="btn">🔙 Back</a>'
puts '<a href="/" class="btn">🏠 Home</a>'
puts '<br><br>'
puts '<small>Last updated: ' + Time.now.strftime("%Y-%m-%d %H:%M:%S") + '</small>'
puts '</div>'
puts '</body>'
puts '</html>'