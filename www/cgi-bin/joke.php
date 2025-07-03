#!/usr/bin/env php-cgi
<?php

## Enhanced PHP Joke Generator with beautiful styling
## echo "Content-Type: text/html; charset=utf-8\r\n";
## echo "\r\n"; // End of headers

// Array of programming jokes with categories
$jokes = array(
	"classic" => array(
		"Why don't programmers like nature? It has too many bugs! 🐛",
		"How many programmers does it take to change a light bulb? None, that's a hardware issue! 💡",
		"Why do Java developers wear glasses? Because they can't C#! 👓",
		"What do you call a programmer from Finland? Nerdic! 🇫🇮",
		"Why did the PHP developer quit his job? Because he didn't get arrays! 📊"
	),
	"dad_jokes" => array(
		"Why was the JavaScript developer sad? Because he didn't know how to 'null' his feelings! 😢",
		"What's the object-oriented way to become wealthy? Inheritance! 💰",
		"Why don't programmers like to go outside? The sun gives them arrays! ☀️",
		"How do you comfort a JavaScript bug? You console it! 🎮",
		"Why did the programmer break up with the internet? Because it couldn't commit! 💔"
	),
	"nerdy" => array(
		"There are only 10 types of people in this world: those who understand binary and those who don't! 01101000",
		"Why do programmers prefer dark mode? Because light attracts bugs! 🌙",
		"What's a programmer's favorite hangout place? Foo Bar! 🍺",
		"Why did the developer go broke? Because he used up all his cache! 💸",
		"What do you call a sleeping bull at a computer? A bulldozer! 😴"
	)
);

// Pick random category and joke
$categories = array_keys($jokes);
$random_category = $categories[array_rand($categories)];
$random_joke = $jokes[$random_category][array_rand($jokes[$random_category])];
$joke_number = rand(1, 1000);

// Fun programming facts
$fun_facts = array(
	"The first computer bug was an actual bug - a moth trapped in a relay!",
	"The term 'debugging' was coined by Grace Hopper in 1947!",
	"PHP originally stood for 'Personal Home Page'!",
	"JavaScript has nothing to do with Java - it's like 'car' and 'carpet'!",
	"The first computer programmer was Ada Lovelace in 1843!"
);

$random_fact = $fun_facts[array_rand($fun_facts)];
?>
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>PHP Joke Generator</title>
	<style>
		* {
			margin: 0;
			padding: 0;
			box-sizing: border-box;
		}

		body {
			font-family: 'Comic Neue', 'Comic Sans MS', cursive, sans-serif;
			background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
			min-height: 100vh;
			padding: 20px;
			display: flex;
			align-items: center;
			justify-content: center;
		}

		.joke-container {
			max-width: 1100px;
			width: 100%;
			background: rgba(255, 255, 255, 0.95);
			border-radius: 30px;
			padding: 50px;
			box-shadow: 0 20px 40px rgba(0, 0, 0, 0.2);
			position: relative;
			overflow: hidden;
			border: 3px solid #667eea;
		}

		.joke-container::before {
			content: '';
			position: absolute;
			top: -50%;
			left: -50%;
			width: 200%;
			height: 200%;
			background: radial-gradient(circle, rgba(102, 126, 234, 0.1) 0%, transparent 70%);
			animation: gentle-rotate 12s linear infinite;
			pointer-events: none;
		}

		@keyframes gentle-rotate {
			0% { transform: rotate(0deg); }
			100% { transform: rotate(360deg); }
		}

		.header {
			text-align: center;
			margin-bottom: 40px;
			position: relative;
			z-index: 1;
		}

		.laugh-emoji {
			font-size: 5rem;
			margin-bottom: 20px;
			animation: laugh-bounce 2s ease-in-out infinite;
			text-shadow: 0 0 20px rgba(102, 126, 234, 0.5);
		}

		@keyframes laugh-bounce {
			0%, 100% { transform: scale(1) rotate(0deg); }
			25% { transform: scale(1.1) rotate(-5deg); }
			50% { transform: scale(1.2) rotate(0deg); }
			75% { transform: scale(1.1) rotate(5deg); }
		}

		h1 {
			color: #667eea;
			font-size: 2.5rem;
			margin-bottom: 15px;
			position: relative;
		}

		.sparkle {
			position: absolute;
			color: #764ba2;
			animation: sparkle 2s ease-in-out infinite;
		}

		.sparkle-1 { top: -10px; left: -20px; animation-delay: 0s; }
		.sparkle-2 { top: -15px; right: -28px; animation-delay: 0.5s; }

		@keyframes sparkle {
			0%, 100% { opacity: 0.3; transform: scale(1); }
			50% { opacity: 1; transform: scale(1.3); }
		}

		.joke-section {
			background: linear-gradient(135deg, #fff5f5 0%, #f0f8ff 100%);
			border-radius: 20px;
			padding: 30px;
			margin: 30px 0;
			border: 3px solid #ffb6c1;
			text-align: center;
			position: relative;
			z-index: 1;
			min-height: 120px;
			display: flex;
			align-items: center;
			justify-content: center;
		}

		.joke-text {
			font-size: 1.3rem;
			color: #333;
			line-height: 1.6;
			font-weight: bold;
		}

		.joke-meta {
			display: flex;
			justify-content: space-between;
			align-items: center;
			margin: 20px 0;
			padding: 15px;
			background: rgba(102, 126, 234, 0.1);
			border-radius: 15px;
			position: relative;
			z-index: 1;
		}

		.joke-category {
			background: #667eea;
			color: white;
			padding: 8px 15px;
			border-radius: 20px;
			font-size: 0.9rem;
			font-weight: bold;
		}

		.joke-number {
			color: #764ba2;
			font-weight: bold;
		}

		.fun-fact-section {
			background: linear-gradient(135deg, #fff9c4 0%, #f093fb 100%);
			border: 3px dashed #ff69b4;
			border-radius: 20px;
			padding: 25px;
			margin: 25px 0;
			position: relative;
			z-index: 1;
		}

		.action-buttons {
			display: flex;
			gap: 15px;
			justify-content: center;
			flex-wrap: wrap;
			margin: 30px 0;
			position: relative;
			z-index: 1;
		}

		.btn {
			padding: 15px 30px;
			border: none;
			border-radius: 25px;
			font-size: 1rem;
			font-weight: bold;
			cursor: pointer;
			text-decoration: none;
			display: inline-block;
			transition: all 0.3s ease;
			font-family: inherit;
		}

		.btn-primary {
			background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
			color: white;
		}

		.btn-secondary {
			background: white;
			color: #667eea;
			border: 2px solid #667eea;
		}

		.btn:hover {
			transform: translateY(-3px) scale(1.05);
			box-shadow: 0 8px 25px rgba(0, 0, 0, 0.2);
		}

		.floating-emojis {
			position: absolute;
			font-size: 2rem;
			opacity: 0.3;
			animation: float-around 20s ease-in-out infinite;
			pointer-events: none;
		}

		@keyframes float-around {
			0%, 100% { transform: translateY(0) rotate(0deg); }
			25% { transform: translateY(-30px) rotate(90deg); }
			50% { transform: translateY(-15px) rotate(180deg); }
			75% { transform: translateY(-35px) rotate(270deg); }
		}

		.rating-section {
			text-align: center;
			margin: 25px 0;
			position: relative;
			z-index: 1;
		}

		.star {
			font-size: 2rem;
			color: #ddd;
			cursor: pointer;
			transition: color 0.3s ease;
		}

		.star:hover,
		.star.active {
			color: #ffd700;
		}
	</style>
</head>
<body>
	<!-- Floating emojis -->
	<div class="floating-emojis" style="top: 10%; left: 10%; animation-delay: 0s;">😂</div>
	<div class="floating-emojis" style="top: 20%; right: 15%; animation-delay: 3s;">🤣</div>
	<div class="floating-emojis" style="bottom: 25%; left: 20%; animation-delay: 6s;">😄</div>
	<div class="floating-emojis" style="bottom: 15%; right: 25%; animation-delay: 9s;">😁</div>

	<div class="joke-container">
		<div class="header">
			<div class="laugh-emoji">😂</div>
			<h1>
				<span class="sparkle sparkle-1">✨</span>
				PHP Joke Generator
				<span class="sparkle sparkle-2">✨</span>
			</h1>
			<p style="color: #666; font-size: 1.2rem;">Powered by PHP and pure silliness! 🎭</p>
		</div>

		<div class="joke-section">
			<div class="joke-text"><?= htmlspecialchars($random_joke) ?></div>
		</div>

		<div class="joke-meta">
			<div class="joke-category"><?= ucfirst(str_replace('_', ' ', $random_category)) ?> Joke</div>
			<div class="joke-number">Joke #<?= $joke_number ?></div>
		</div>

		<div class="rating-section">
			<p style="margin-bottom: 10px; color: #666;">Rate this joke:</p>
			<div onclick="rateJoke(this)">
				<span class="star" data-rating="1">⭐</span>
				<span class="star" data-rating="2">⭐</span>
				<span class="star" data-rating="3">⭐</span>
				<span class="star" data-rating="4">⭐</span>
				<span class="star" data-rating="5">⭐</span>
			</div>
		</div>

		<div class="fun-fact-section">
			<h3 style="color: #ff1493; margin-bottom: 15px; text-align: center;">🤓 Fun Programming Fact!</h3>
			<p style="margin: 0; color: #333; text-align: center; font-size: 1.1rem; line-height: 1.5;">
				<?= htmlspecialchars($random_fact) ?>
			</p>
		</div>

		<div class="action-buttons">
			<form method="GET" action="/cgi-bin/joke.php" style="display: inline;">
				<button type="submit" class="btn btn-primary">🎲 Another Joke!</button>
			</form>
			<a href="/web/playground.html" class="btn btn-secondary">🔙 Back to Playground</a>
			<a href="/" class="btn btn-secondary">🏠 Home</a>
		</div>

		<div style="text-align: center; margin-top: 30px; color: #999; font-style: italic; position: relative; z-index: 1;">
			😂 Laughs powered by PHP and WebServ! Keep coding and keep smiling! ✨
		</div>
	</div>

	<script>
		// Interactive rating system
		function rateJoke(container) {
			const stars = container.querySelectorAll('.star');

			stars.forEach(star => {
				star.addEventListener('click', function() {
					const rating = parseInt(this.dataset.rating);

					// Reset all stars
					stars.forEach(s => s.classList.remove('active'));

					// Activate stars up to the clicked one
					for (let i = 0; i < rating; i++) {
						stars[i].classList.add('active');
					}

					// Show thank you message
					showThankYou(rating);
				});
			});
		}

		function showThankYou(rating) {
			const message = document.createElement('div');
			const responses = [
				'', // 0 - not used
				'😅 Tough crowd! Thanks for the feedback!',
				'😊 Could be better, we get it!',
				'😄 Not bad! Thanks for rating!',
				'😂 Great! Glad you enjoyed it!',
				'🤣 Awesome! You made our day!'
			];

			message.style.cssText = `
				position: fixed;
				top: 20px;
				right: 20px;
				background: linear-gradient(135deg, #667eea, #764ba2);
				color: white;
				padding: 15px 25px;
				border-radius: 25px;
				z-index: 1000;
				animation: slideIn 0.5s ease-out;
				box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
				font-weight: bold;
			`;
			message.textContent = responses[rating];

			document.body.appendChild(message);

			setTimeout(() => {
				message.style.animation = 'slideOut 0.5s ease-in forwards';
				setTimeout(() => message.remove(), 500);
			}, 3000);
		}

		// Floating laugh effects
		function createLaugh() {
			const laughs = ['😂', '🤣', '😄', '😁', '😆'];
			const laugh = document.createElement('div');
			laugh.innerHTML = laughs[Math.floor(Math.random() * laughs.length)];
			laugh.style.cssText = `
				position: fixed;
				left: ${Math.random() * window.innerWidth}px;
				top: ${window.innerHeight + 20}px;
				font-size: ${20 + Math.random() * 20}px;
				pointer-events: none;
				z-index: 9999;
				animation: float-up 4s ease-out forwards;
				color: #667eea;
			`;

			document.body.appendChild(laugh);
			setTimeout(() => laugh.remove(), 4000);
		}

		// Create floating laughs periodically
		setInterval(createLaugh, 2000);

		// Add CSS for animations
		const style = document.createElement('style');
		style.textContent = `
			@keyframes slideIn {
				from { transform: translateX(100%); opacity: 0; }
				to { transform: translateX(0); opacity: 1; }
			}
			@keyframes slideOut {
				from { transform: translateX(0); opacity: 1; }
				to { transform: translateX(100%); opacity: 0; }
			}
			@keyframes float-up {
				0% { transform: translateY(0) rotate(0deg); opacity: 0.8; }
				100% { transform: translateY(-200px) rotate(360deg); opacity: 0; }
			}
		`;
		document.head.appendChild(style);

		// Initialize rating system
		document.addEventListener('DOMContentLoaded', function() {
			const ratingContainer = document.querySelector('.rating-section > div');
			if (ratingContainer) {
				rateJoke(ratingContainer);
			}
		});
	</script>
</body>
</html>