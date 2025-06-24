#!/usr/bin/env python3
"""
🫖 Coffee Brewing Script - Triggers HTTP 418 I'm a teapot 🫖
Because sometimes a teapot just can't brew coffee!
Replace your badscript.py with this file.
"""

import sys
import os
import time
from datetime import datetime

# Print the magical 418 status
print("Status: 418 I'm a teapot\r")
print("Content-Type: text/html; charset=utf-8\r")
print("X-Brew-Error: Cannot brew coffee with a teapot\r")
print("X-Tea-Ready: Always available\r")
print("Retry-After: Never (I'm permanently a teapot)\r")
print("\r")  # End headers

# Get current time for brewing attempt timestamp
current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>418 - I'm a Teapot</title>
	<style>
		@import url('https://fonts.googleapis.com/css2?family=Fredoka+One:wght@400&family=Comic+Neue:wght@400;700&display=swap');

		body.error-418 {{
			background: linear-gradient(135deg, #8B4513 0%, #DEB887 50%, #F5DEB3 100%);
			min-height: 100vh;
			display: flex;
			align-items: center;
			justify-content: center;
		}}

		.teapot-container {{
			background: rgba(255, 255, 255, 0.95);
			border-radius: 30px;
			padding: 50px;
			max-width: 1200px;
			text-align: center;
			box-shadow: 0 20px 40px rgba(139, 69, 19, 0.3);
			position: relative;
			overflow: hidden;
		}}

		.teapot-container::before {{
			content: '';
			position: absolute;
			top: -50%;
			right: -50%;
			width: 200%;
			height: 200%;
			pointer-events: none;
			background: radial-gradient(circle, rgba(222, 184, 135, 0.1) 0%, transparent 70%);
			animation: steam 8s ease-in-out infinite;
		}}

		@keyframes steam {{
			0%, 100% {{ transform: translate(0, 0) rotate(0deg); }}
			25% {{ transform: translate(-10px, -20px) rotate(90deg); }}
			50% {{ transform: translate(-5px, -40px) rotate(180deg); }}
			75% {{ transform: translate(10px, -20px) rotate(270deg); }}
		}}

		/* Floating coffee beans */
		.coffee-bean {{
			position: absolute;
			font-size: 2rem;
			opacity: 0.3;
			animation: float-beans 15s linear infinite;
			pointer-events: none;
		}}

		.teapot-emoji {{
			font-size: 8rem;
			margin: 20px 0;
			animation: teapot-wiggle 3s ease-in-out infinite;
			text-shadow: 0 5px 15px rgba(139, 69, 19, 0.3);
		}}

		@keyframes teapot-wiggle {{
			0%, 100% {{ transform: rotate(0deg) scale(1); }}
			25% {{ transform: rotate(-5deg) scale(1.05); }}
			50% {{ transform: rotate(0deg) scale(1.1); }}
			75% {{ transform: rotate(5deg) scale(1.05); }}
		}}

		.explanation {{
				background: linear-gradient(135deg, #F5DEB3 0%, rgb(255, 241, 223) 100%);
				border: 3px dashed #8B4513;
				border-radius: 20px;
				padding: 25px;
				margin: 30px 0;
				position: relative;
				z-index: 1;
			}}

		.error-code {{
			font-size: 4rem;
			color: #8B4513;
			font-weight: bold;
			margin: 20px 0 10px 0;
			text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.1);
		}}

		.error-title {{
			font-size: 2.5rem;
			color: #A0522D;
			margin: 10px 0 20px 0;
			font-weight: bold;
		}}

		.teapot-message {{
			font-size: 1.2rem;
			line-height: 1.6;
			color: #654321;
			margin: 20px 0 30px 0;
		}}

		/* Steam effect */
		.steams {{
			position: absolute;
			font-size: 1.5rem;
			opacity: 0.6;
			animation: steam-rise 3s ease-in-out infinite;
			pointer-events: none;
		}}

		@keyframes steams-rise {{
			0% {{ transform: translateY(0) scale(1); opacity: 0.6; }}
			50% {{ transform: translateY(-30px) scale(1.2); opacity: 0.3; }}
			100% {{ transform: translateY(-60px) scale(1.5); opacity: 0; }}
		}}

		.steam-effect {{
			position: absolute;
			top: 20px;
			left: 50%;
			transform: translateX(-50%);
			font-size: 1.5rem;
			opacity: 0.6;
			animation: float-steam 4s ease-in-out infinite;
			pointer-events: none;
		}}

		@keyframes float-steam {{
			0%, 100% {{ transform: translateX(-50%) translateY(0); opacity: 0.6; }}
			50% {{ transform: translateX(-50%) translateY(-20px); opacity: 0.3; }}
		}}

		.coffee-options {{
			background: linear-gradient(135deg, #F5DEB3 0%, #DEB887 100%);
			border: 3px solid #CD853F;
			border-radius: 20px;
			padding: 25px;
			margin: 30px 0;
		}}

		.coffee-options h3 {{
			color: #8B4513;
			margin-bottom: 15px;
		}}

		.coffee-buttons {{
			display: flex;
			gap: 15px;
			justify-content: center;
			flex-wrap: wrap;
			margin-top: 20px;
		}}

		.coffee-btn {{
			background: linear-gradient(135deg, #228B22 0%, #32CD32 100%);
			color: white;
			border: none;
			padding: 12px 20px;
			border-radius: 20px;
			cursor: pointer;
			font-weight: bold;
			transition: all 0.3s ease;
			display: flex;
			align-items: center;
			gap: 8px;
		}}

		.coffee-btn:hover {{
			transform: translateY(-3px) scale(1.05);
			box-shadow: 0 8px 20px rgba(139, 69, 19, 0.4);
		}}

		.teapot-facts {{
			background: linear-gradient(135deg, #FFF8DC 0%, #F0E68C 100%);
			border: 3px dashed #DAA520;
			border-radius: 15px;
			padding: 20px;
			margin: 25px 0;
			text-align: left;
		}}

		.teapot-facts h4 {{
			color: #B8860B;
			text-align: center;
			margin-bottom: 15px;
		}}

		.teapot-facts ul {{
			color: #8B7355;
			line-height: 1.6;
		}}

		.teapot-facts li {{
			margin: 8px 0;
		}}

		.action-buttons {{
			display: flex;
			gap: 15px;
			justify-content: center;
			margin-top: 30px;
			flex-wrap: wrap;
		}}

		.home-link.teapot {{
			background: linear-gradient(135deg, #8B4513 0%, #A0522D 100%);
			color: white;
			padding: 12px 25px;
			border-radius: 25px;
			text-decoration: none;
			font-weight: bold;
			transition: all 0.3s ease;
			display: flex;
			align-items: center;
			gap: 8px;
		}}

		.home-link.teapot:hover {{
			transform: translateY(-3px);
			box-shadow: 0 8px 20px rgba(139, 69, 19, 0.4);
		}}

		.brewing-animation {{
			position: relative;
			display: inline-block;
		}}

		.brewing-dots {{
			display: inline-block;
			animation: brewing 2s ease-in-out infinite;
		}}

		@keyframes brewing {{
			0%, 100% {{ opacity: 1; }}
			50% {{ opacity: 0.3; }}
		}}

		.timestamp {{
			color: #8B4513;
			font-style: italic;
			font-size: 0.9rem;
			margin-top: 20px;
			position: relative;
			z-index: 1;
		}}

	</style>
</head>
<body class="error-418">

	<!-- Floating coffee beans -->
	<div class="coffee-bean" style="left: 10%; animation-delay: 0s;">☕</div>
	<div class="coffee-bean" style="left: 20%; animation-delay: 2s;">☕</div>
	<div class="coffee-bean" style="left: 80%; animation-delay: 4s;">☕</div>
	<div class="coffee-bean" style="left: 90%; animation-delay: 6s;">☕</div>
	<div class="steam-effect">☁️ ☁️ ☁️</div>

	<div class="teapot-container">
		<!-- Steam effects -->
		<div class="steams" style="top: 60px; left: 45%; animation-delay: 0s;">💨</div>
		<div class="steams" style="top: 60px; left: 50%; animation-delay: 1s;">💨</div>
		<div class="steams" style="top: 60px; left: 55%; animation-delay: 2s;">💨</div>

		<div class="teapot-emoji">🫖</div>
		<div class="error-code">ERROR 418</div>
		<div class="error-title">I'm a Teapot!</div>

		<div class="teapot-message">
			<p><strong>Oops! I'm a teapot, not a coffee machine!</strong> ☕</p>
			<p>☕ Coffee brewing request denied ☕</p>
		</div>

		<div class="explanation">
			<h3 style="color: #8B4513; margin-bottom: 15px;">📜 What happened?</h3>
			<p style="margin: 0; color: #654321; line-height: 1.5;">
				You tried to ask me to brew coffee, but I'm a teapot! According to RFC 2324
				(Hyper Text Coffee Pot Control Protocol), teapots should return a 418 status
				when asked to brew coffee. It's not my fault - it's just physics! 🫖✨
			</p>
		</div>

		<div class="coffee-options">
			<h3>🍃 But I can offer you some lovely tea instead!</h3>
			<p style="margin: 10px 0; color: #654321;">Choose your favorite blend:</p>

			<div class="coffee-buttons">
				<button class="coffee-btn" onclick="brewTea('Earl Grey')">
					🫖 Earl Grey
				</button>
				<button class="coffee-btn" onclick="brewTea('Green Tea')">
					🍃 Green Tea
				</button>
				<button class="coffee-btn" onclick="brewTea('Chamomile')">
					🌼 Chamomile
				</button>
				<button class="coffee-btn" onclick="brewTea('Chai')">
					🧡 Chai Spice
				</button>
				<button class="coffee-btn" onclick="brewTea('Peppermint')">
					🌿 Peppermint
				</button>
				<button class="coffee-btn" onclick="brewTea('Jasmine')">
					🌸 Oolong
				</button>
				<button class="coffee-btn" onclick="brewTea('Surprise')">
					🎲 Surprise Tea
				</button>
			</div>
		</div>

		<div class="teapot-facts">
			<h4>🤓 Fun Facts About HTTP 418:</h4>
			<ul>
				<li>📅 Created in 1998 as part of RFC 2324 (April Fool's Day)</li>
				<li>🎭 Originally part of the "Hyper Text Coffee Pot Control Protocol"</li>
				<li>🌐 Many servers actually implement this error code!</li>
				<li>☕ It's meant to be returned when someone tries to brew coffee with a teapot</li>
				<li>💖 It shows that programmers have a sense of humor!</li>
			</ul>
		</div>

		<div id="brewing-message" style="display: none; color: #8B4513; font-weight: bold; margin: 20px 0;">
			<div class="brewing-animation">
				☕ Brewing your tea<span class="brewing-dots">...</span>
			</div>
		</div>

		<div class="action-buttons">
			<a href="javascript:history.back()" class="home-link teapot">
				← Go Back
			</a>
			<a href="/" class="home-link teapot" onclick="goHome(event)">
				🏠 Go Home
			</a>
			<button class="home-link teapot" onclick="findCoffee()" style="border: none;">
				☕ Find Coffee Instead
			</button>
		</div>

		<div class="timestamp">
			Brewing attempt failed at: {current_time}<br>
			🫖 Still proudly a teapot since 1998 🫖
		</div>
	</div>

	<script>
		function brewTea(teaType) {{
			const messages = {{
				'surprise': '🎲 Surprise tea: Magical blend with extra sparkles! ✨',
				'Earl Grey': '🫖 Perfect Earl Grey brewing... Elegant and bergamot-scented! 🍋',
				'Green Tea': '🍃 Fresh green tea steeping... Zen and healthy! 🧘‍♀️',
				'Chamomile': '🌼 Soothing chamomile brewing... Perfect for relaxation! 😌',
				'Chai': '🧡 Spicy chai tea brewing... Warm and comforting! 🍂',
				'Peppermint': '🌿 Refreshing peppermint tea brewing... Cool and invigorating! ❄️',
				'Jasmine': '🌸 Fragrant jasmine tea brewing... Floral and delightful! 🌺'
			}};

			const message = messages[teaType] || '🫖 Mystery tea brewing... Surprise flavor incoming! 🎭';

			// Create brewing notification
			const notification = document.createElement('div');
			notification.style.cssText = `
				position: fixed;
				top: 20px;
				right: 20px;
				background: linear-gradient(135deg, #228B22, #32CD32);
				color: white;
				padding: 20px 30px;
				border-radius: 25px;
				z-index: 10000;
				animation: slideIn 0.5s ease-out;
				box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
				font-weight: bold;
				max-width: 350px;
				border: 3px solid #90EE90;
			`;

			notification.innerHTML = `
				<div style="font-size: 1.2rem; margin-bottom: 5px;">🫖 Tea Brewing Success!</div>
				<div style="font-size: 1rem;">${{message}}</div>
			`;

			document.body.appendChild(notification);

			// Add sparkle effect
			createSparkles();

			setTimeout(() => {{
				notification.style.animation = 'slideOut 0.5s ease-in forwards';
				setTimeout(() => notification.remove(), 500);
			}}, 4000);
		}}

		function createSparkles() {{
			for (let i = 0; i < 20; i++) {{
				setTimeout(() => {{
					const sparkle = document.createElement('div');
					sparkle.innerHTML = '✨';
					sparkle.style.cssText = `
						position: fixed;
						left: ${{Math.random() * window.innerWidth}}px;
						top: ${{Math.random() * window.innerHeight}}px;
						font-size: ${{15 + Math.random() * 15}}px;
						pointer-events: none;
						z-index: 9999;
						animation: sparkle-magic 3s ease-out forwards;
						color: #32CD32;
					`;

					document.body.appendChild(sparkle);
					setTimeout(() => sparkle.remove(), 3000);
				}}, i * 100);
			}}
		}}

		// Add floating coffee scent effect
		function createCoffeeScent() {{
			const scent = document.createElement('div');
			scent.innerHTML = '☁️';
			scent.style.cssText = `
				position: fixed;
				left: ${{Math.random() * window.innerWidth}}px;
				top: ${{window.innerHeight + 20}}px;
				font-size: ${{20 + Math.random() * 10}}px;
				pointer-events: none;
				z-index: 1000;
				animation: denied-scent 6s ease-out forwards;
				opacity: 0.7;
			`;

			document.body.appendChild(scent);
			setTimeout(() => scent.remove(), 6000);
		}}

		// Create denied coffee scents periodically
		setInterval(createCoffeeScent, 3000);

		// Add CSS animations
		const style = document.createElement('style');
		style.textContent = `
			@keyframes slideIn {{
				from {{ transform: translateX(100%); opacity: 0; }}
				to {{ transform: translateX(0); opacity: 1; }}
			}}
			@keyframes slideOut {{
				from {{ transform: translateX(0); opacity: 1; }}
				to {{ transform: translateX(100%); opacity: 0; }}
			}}
			@keyframes sparkle-magic {{
				0% {{ opacity: 1; transform: scale(0) rotate(0deg); }}
				50% {{ opacity: 1; transform: scale(1) rotate(180deg); }}
				100% {{ opacity: 0; transform: scale(0) rotate(360deg); }}
			}}
			@keyframes denied-scent {{
				0% {{ transform: translateY(0) rotate(0deg); opacity: 0.7; }}
				50% {{ transform: translateY(-100px) rotate(180deg); opacity: 0.4; }}
				100% {{ transform: translateY(-200px) rotate(360deg); opacity: 0; }}
			}}
		`;
		document.head.appendChild(style);

		// Welcome message for teapot enthusiasts
		setTimeout(() => {{
			console.log('🫖 Welcome to the most polite error page on the internet! 🫖');
			console.log('📜 RFC 2324 compliance level: 100%');
			console.log('☕ Coffee brewing capability: 0%');
			console.log('🍃 Tea brewing capability: ∞%');
		}}, 1000);
	</script>
</body>
</html>
"""

# Output the magical HTML!
print(html_content)