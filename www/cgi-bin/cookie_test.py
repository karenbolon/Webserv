#!/usr/bin/env python3

import sys
import os
from datetime import datetime
import time

#for infinite loop testing CGI
#while True:
#    time.sleep(1)

# Get environment variables
cookie = os.environ.get("HTTP_COOKIE", "")
query = os.environ.get("QUERY_STRING", "")
headers = []

# Debug print to stderr
print("DEBUG - Cookie received:", repr(cookie), file=sys.stderr)
print("DEBUG - Query string:", repr(query), file=sys.stderr)

# Check if this is a reset request
if "reset=1" in query:
	# Clear all cookies
	headers.append("Set-Cookie: visited=; Path=/; Max-Age=0")
	headers.append("Set-Cookie: visit_count=; Path=/; Max-Age=0")
	headers.append("Set-Cookie: last_visit=; Path=/; Max-Age=0")
	is_returning = False
	visit_count = 0
	message = "🧹 Cookies cleared! You're a fresh visitor again!"
	print("DEBUG - Resetting cookies", file=sys.stderr)
else:
	# Parse existing cookies
	is_returning = "visited=yes" in cookie

	# Get visit count - better parsing
	visit_count = 1
	if "visit_count=" in cookie:
		try:
			# Extract visit count from cookie string
			for part in cookie.split(';'):
				part = part.strip()  # Remove whitespace
				if part.startswith('visit_count='):
					current_count = int(part.split('=')[1])
					visit_count = current_count + 1
					print(f"DEBUG - Found existing count: {current_count}, incrementing to: {visit_count}", file=sys.stderr)
					break
		except (ValueError, IndexError) as e:
			print(f"DEBUG - Error parsing visit count: {e}", file=sys.stderr)
			visit_count = 1

	print(f"DEBUG - Is returning: {is_returning}, Visit count: {visit_count}", file=sys.stderr)

	# Set cookies based on status
	if not is_returning:
		# First time visitor
		headers.append("Set-Cookie: visited=yes; Path=/; Max-Age=86400")  # 24 hours
		headers.append("Set-Cookie: visit_count=1; Path=/; Max-Age=86400")
		headers.append(f"Set-Cookie: last_visit={datetime.now().isoformat()}; Path=/; Max-Age=86400")
		message = "🎉 Welcome! Your first magical cookie has been created!"
		visit_count = 1
		print("DEBUG - Setting first time cookies", file=sys.stderr)
	else:
		# Returning visitor - increment count
		headers.append("Set-Cookie: visited=yes; Path=/; Max-Age=86400")  # Refresh the cookie
		headers.append(f"Set-Cookie: visit_count={visit_count}; Path=/; Max-Age=86400")
		headers.append(f"Set-Cookie: last_visit={datetime.now().isoformat()}; Path=/; Max-Age=86400")
		message = f"🎉 Welcome back! This is your visit #{visit_count}!"
		print(f"DEBUG - Setting returning visitor cookies, count: {visit_count}", file=sys.stderr)

# Add content type header
headers.append("Content-Type: text/html; charset=utf-8")

# Send all headers
for h in headers:
	sys.stdout.write(h + "\r\n")
sys.stdout.write("\r\n")  # End of headers

# Determine theme based on visit count
if visit_count == 1:
	theme_color = "#FF69B4"
	bg_gradient = "linear-gradient(135deg, #FFE4E1 0%, #FFF0F5 50%, #F0F8FF 100%)"
	icon = "🍪"
elif visit_count <= 5:
	theme_color = "#9370DB"
	bg_gradient = "linear-gradient(135deg, #E6E6FA 0%, #F0F8FF 50%, #FFF8DC 100%)"
	icon = "🍪✨"
elif visit_count <= 10:
	theme_color = "#FF6347"
	bg_gradient = "linear-gradient(135deg, #FFD700 0%, #FFA500 50%, #FF6347 100%)"
	icon = "🍪👑"
else:
	theme_color = "#8A2BE2"
	bg_gradient = "linear-gradient(135deg, #8A2BE2 0%, #4B0082 50%, #9400D3 100%)"
	icon = "🍪🏆✨"

# Generate HTML with beautiful styling
html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Cookie Magic Test</title>
	<style>
		@import url('https://fonts.googleapis.com/css2?family=Quicksand:wght@400;600;700&family=Fredoka+One&display=swap');

		* {{
			margin: 0;
			padding: 0;
			box-sizing: border-box;
		}}

		body {{
			font-family: 'Quicksand', sans-serif;
			background: {bg_gradient};
			background-size: 400% 400%;
			animation: magical-gradient 10s ease-in-out infinite;
			min-height: 100vh;
			display: flex;
			align-items: center;
			justify-content: center;
			padding: 20px;
			overflow-x: hidden;
			position: relative;
		}}

		@keyframes magical-gradient {{
			0%, 100% {{ background-position: 0% 50%; }}
			50% {{ background-position: 100% 50%; }}
		}}

		/* Floating cookie crumbs */
		.cookie-crumb {{
			position: absolute;
			font-size: 2rem;
			opacity: 0.6;
			animation: float-crumbs 15s linear infinite;
			pointer-events: none;
		}}

		@keyframes float-crumbs {{
			0% {{ transform: translateY(100vh) translateX(0) rotate(0deg); }}
			100% {{ transform: translateY(-100px) translateX(50px) rotate(360deg); }}
		}}

		.cookie-container {{
			max-width: 900px;
			width: 100%;
			background: rgba(255, 255, 255, 0.95);
			border-radius: 30px;
			padding: 50px;
			text-align: center;
			box-shadow: 0 25px 50px rgba(0, 0, 0, 0.2);
			border: 4px solid {theme_color};
			position: relative;
			backdrop-filter: blur(15px);
		}}

		.cookie-container::before {{
			content: '';
			position: absolute;
			top: -50%;
			left: -50%;
			width: 200%;
			height: 200%;
			background: radial-gradient(circle, {theme_color}22 0%, transparent 70%);
			animation: magical-swirl 20s linear infinite;
			pointer-events: none;
		}}

		@keyframes magical-swirl {{
			0% {{ transform: rotate(0deg); }}
			100% {{ transform: rotate(360deg); }}
		}}

		.cookie-icon {{
			font-size: 5rem;
			margin: 20px 0;
			animation: cookie-bounce 2.5s ease-in-out infinite;
			text-shadow: 0 0 20px {theme_color};
			position: relative;
			z-index: 1;
		}}

		@keyframes cookie-bounce {{
			0%, 100% {{ transform: translateY(0) scale(1); }}
			50% {{ transform: translateY(-15px) scale(1.1); }}
		}}

		h1 {{
			font-family: 'Fredoka One', cursive;
			color: {theme_color};
			font-size: 2.2rem;
			margin: 25px 0;
			text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.1);
			position: relative;
			z-index: 1;
		}}

		.message-box {{
			background: linear-gradient(135deg, rgba(255, 255, 255, 0.8) 0%, rgba(255, 240, 245, 0.9) 100%);
			border-radius: 20px;
			padding: 25px;
			margin: 25px 0;
			border: 3px solid {theme_color};
			position: relative;
			z-index: 1;
			font-size: 1.2rem;
			color: #333;
			font-weight: 600;
			line-height: 1.6;
		}}

		.stats-grid {{
			display: grid;
			grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
			gap: 15px;
			margin: 25px 0;
			position: relative;
			z-index: 1;
		}}

		.stat-card {{
			background: linear-gradient(135deg, #fff 0%, #f8f9ff 100%);
			border-radius: 15px;
			padding: 15px;
			border: 2px solid {theme_color};
			transition: all 0.3s ease;
		}}

		.stat-card:hover {{
			transform: translateY(-5px) scale(1.05);
			box-shadow: 0 10px 20px rgba(0, 0, 0, 0.2);
		}}

		.stat-icon {{
			font-size: 2rem;
			margin-bottom: 8px;
		}}

		.stat-label {{
			color: #666;
			font-size: 0.8rem;
			margin-bottom: 5px;
			font-weight: 600;
		}}

		.stat-value {{
			color: {theme_color};
			font-size: 1.1rem;
			font-weight: 700;
		}}

		.cookie-display {{
			background: rgba(0, 0, 0, 0.05);
			border-radius: 15px;
			padding: 20px;
			margin: 20px 0;
			border: 2px dashed {theme_color};
			position: relative;
			z-index: 1;
		}}

		.cookie-title {{
			color: {theme_color};
			font-weight: 700;
			margin-bottom: 10px;
		}}

		.cookie-data {{
			font-family: 'Courier New', monospace;
			font-size: 0.85rem;
			color: #555;
			line-height: 1.4;
			max-height: 80px;
			overflow-y: auto;
			word-break: break-all;
		}}

		.action-buttons {{
			display: flex;
			gap: 15px;
			justify-content: center;
			flex-wrap: wrap;
			margin: 30px 0;
			position: relative;
			z-index: 1;
		}}

		.btn {{
			padding: 15px 25px;
			border: none;
			border-radius: 20px;
			font-size: 1rem;
			font-weight: 600;
			cursor: pointer;
			text-decoration: none;
			display: inline-block;
			transition: all 0.3s ease;
			font-family: inherit;
		}}

		.btn-clear {{
			background: linear-gradient(135deg, #FF6B6B 0%, #FF8E8E 100%);
			color: white;
		}}

		.btn-refresh {{
			background: linear-gradient(135deg, {theme_color} 0%, {theme_color}cc 100%);
			color: white;
		}}

		.btn-back {{
			background: white;
			color: {theme_color};
			border: 3px solid {theme_color};
		}}

		.btn:hover {{
			transform: translateY(-3px) scale(1.05);
			box-shadow: 0 10px 25px rgba(0, 0, 0, 0.3);
		}}

		.timestamp {{
			color: #888;
			font-style: italic;
			font-size: 0.9rem;
			margin-top: 20px;
			position: relative;
			z-index: 1;
		}}

		/* Responsive design */
		@media (max-width: 768px) {{
			.cookie-container {{
				padding: 30px 20px;
				margin: 10px;
			}}

			.cookie-icon {{
				font-size: 3.5rem;
			}}

			h1 {{
				font-size: 1.8rem;
			}}

			.action-buttons {{
				flex-direction: column;
				align-items: center;
			}}

			.btn {{
				width: 100%;
				max-width: 250px;
			}}
		}}
	</style>
</head>
<body>
	<!-- Floating cookie crumbs -->
	<div class="cookie-crumb" style="left: 10%; animation-delay: 0s;">🍪</div>
	<div class="cookie-crumb" style="left: 30%; animation-delay: 2s;">✨</div>
	<div class="cookie-crumb" style="left: 60%; animation-delay: 4s;">🍪</div>
	<div class="cookie-crumb" style="left: 85%; animation-delay: 6s;">✨</div>

	<div class="cookie-container">
		<div class="cookie-icon">{icon}</div>

		<h1>🍪 Cookie Magic Test</h1>

		<div class="message-box">
			{message}
		</div>

		<div class="action-buttons">
			<a href="/cgi-bin/cookie_test.py" class="btn btn-refresh">
				🔄 Refresh Page
			</a>
			<a href="/cgi-bin/cookie_test.py?reset=1" class="btn btn-clear"
			onclick="return confirm('🥺 Are you sure you want to clear your cookies?')">
				🧹 Clear Cookies
			</a>
			<a href="/web/playground.html" class="btn btn-back">
				🔙 Back to Playground
			</a>
		</div>

		<div class="timestamp">
			Last updated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}
		</div>
	</div>

	<script>
		// Create magical sparkle effects
		function createSparkles() {{
			for (let i = 0; i < 10; i++) {{
				setTimeout(() => {{
					const sparkle = document.createElement('div');
					sparkle.innerHTML = '✨';
					sparkle.style.cssText = `
						position: fixed;
						left: ${{Math.random() * window.innerWidth}}px;
						top: ${{Math.random() * window.innerHeight}}px;
						font-size: ${{15 + Math.random() * 15}}px;
						pointer-events: none;
						z-index: 1000;
						animation: sparkle-magic 3s ease-out forwards;
						color: {theme_color};
					`;

					document.body.appendChild(sparkle);
					setTimeout(() => sparkle.remove(), 3000);
				}}, i * 100);
			}}
		}}

		// Button effects
		document.querySelectorAll('.btn').forEach(btn => {{
			btn.addEventListener('mouseenter', function() {{
				if (Math.random() > 0.7) {{
					createSparkles();
				}}
			}});
		}});

		// Periodic sparkle effects
		setInterval(() => {{
			if (Math.random() > 0.8) {{
				createSparkles();
			}}
		}}, 5000);

		// Add CSS animations
		const style = document.createElement('style');
		style.textContent = `
			@keyframes sparkle-magic {{
				0% {{ opacity: 1; transform: scale(0) rotate(0deg); }}
				50% {{ opacity: 1; transform: scale(1) rotate(180deg); }}
				100% {{ opacity: 0; transform: scale(0) rotate(360deg); }}
			}}
		`;
		document.head.appendChild(style);

		console.log('🍪 Cookie Magic Test initialized!');
		console.log('Visit count: {visit_count}');
		console.log('Is returning: {is_returning}');
		console.log('Current cookies: {cookie}');
	</script>
</body>
</html>
"""

# Output the HTML
sys.stdout.write(html_content)
