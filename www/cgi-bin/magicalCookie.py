#!/usr/bin/env python3
"""
🍪✨ SUPER CUTE & FUNNY COOKIE MAGIC ✨🍪
The most adorable cookie system ever created for your photo gallery!
With personality, humor, and pure magical cuteness! 🦄💖
"""

import cgi
import cgitb
import os
import sys
import urllib.parse
from datetime import datetime, timedelta
import random
import json

# Enable CGI error reporting for debugging
cgitb.enable()

# Get environment variables
cookie_header = os.environ.get("HTTP_COOKIE", "")
query_string = os.environ.get("QUERY_STRING", "")
request_method = os.environ.get("REQUEST_METHOD", "GET")

def parse_cookies(cookie_string):
    """Parse cookie string into a cute dictionary! 🍪"""
    cookies = {}
    if cookie_string:
        for cookie in cookie_string.split(';'):
            if '=' in cookie:
                key, value = cookie.strip().split('=', 1)
                cookies[key] = urllib.parse.unquote(value)
    return cookies

def parse_query_params(query_string):
    """Parse query parameters with sparkles! ✨"""
    params = {}
    if query_string:
        for param in query_string.split('&'):
            if '=' in param:
                key, value = param.split('=', 1)
                params[key] = urllib.parse.unquote(value)
    return params

def parse_form_data():
    """Parse POST form data with magic! 🪄"""
    form_data = {}
    if request_method == "POST":
        try:
            # Try to handle the form data more carefully
            import sys
            import io

            # Read the content length
            content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
            if content_length > 0:
                # Read the raw data from stdin
                post_data = sys.stdin.buffer.read(content_length).decode('utf-8')

                # Parse the URL-encoded data manually
                if post_data:
                    for pair in post_data.split('&'):
                        if '=' in pair:
                            key, value = pair.split('=', 1)
                            # URL decode the key and value
                            key = urllib.parse.unquote_plus(key)
                            value = urllib.parse.unquote_plus(value)
                            form_data[key] = value
        except Exception as e:
            # Fallback: try the traditional CGI method with error handling
            try:
                # Set up a proper text stream for FieldStorage
                sys.stdin = io.TextIOWrapper(sys.stdin.buffer, encoding='utf-8')
                form = cgi.FieldStorage()
                for key in form.keys():
                    form_data[key] = form.getvalue(key)
            except Exception:
                # If all else fails, just return empty dict
                pass
    return form_data

# Get all the magical data
cookies = parse_cookies(cookie_header)
query_params = parse_query_params(query_string)
form_data = parse_form_data()
params = {**form_data, **query_params}

# 🌟 SUPER CUTE MESSAGES WITH PERSONALITY 🌟
funny_welcome_messages = [
    "🎪 Welcome back, you magnificent human! The cookies missed you SO much they almost cried! 🥺",
    "✨ *GASP* You're back! The server literally did a happy dance! 💃🕺",
    "🦄 A wild visitor appeared! It's super effective at making our day! 🌟",
    "🍪 Breaking news: Local cookie jar reports maximum cuteness levels detected! 📰",
    "🎭 Plot twist: You're not just a visitor, you're our FAVORITE visitor! 🏆",
    "🌈 Warning: Your presence has caused spontaneous rainbow generation! 🌦️",
    "🎯 Achievement unlocked: Making a web server blush! 😊",
    "🚀 Houston, we have a problem... we're TOO excited to see you! 🛸"
]

first_time_messages = [
    "🎉 FIRST TIME VISITOR ALERT! 🚨 Quick, someone bring the confetti cannon! 🎊",
    "✨ Fresh human detected! Initializing maximum cuteness protocols... 🤖💖",
    "🦋 A beautiful butterfly has landed on our digital flower! Welcome! 🌸",
    "🍓 Sweet! A new friend joins our strawberry-scented adventure! 🌟",
    "🎪 Ladies and gentlemen, presenting... OUR NEWEST STAR VISITOR! ⭐",
    "🌙 Once upon a time, a lovely human discovered our magical gallery... 📚✨",
    "🎨 Breaking: Local art gallery gains +1 awesome human! Stats off the charts! 📈",
    "🔮 The crystal ball predicted your arrival! Welcome to the magic! 🪄"
]

# 🎭 PERSONALITY MODES (way funnier than just moods!)
personality_modes = {
    "bubbly": {
        "emoji": "🫧",
        "bg": "#f0f8ff",
        "message": "Bouncing with excitement like soap bubbles!",
        "effect": "bubble-float",
        "sound": "Pop! Pop! Fizz!"
    },
    "giggly": {
        "emoji": "😸",
        "bg": "#fff5ee",
        "message": "Giggling uncontrollably at everything!",
        "effect": "giggle-shake",
        "sound": "*Hehe* *Haha* *Snort*"
    },
    "dramatic": {
        "emoji": "🎭",
        "bg": "#f5f0ff",
        "message": "EVERYTHING is ABSOLUTELY MAGNIFICENT!",
        "effect": "dramatic-flair",
        "sound": "*Gasp* How MARVELOUS!"
    },
    "sleepy": {
        "emoji": "😴",
        "bg": "#f0f0f8",
        "message": "Yaaawn... but still excited to see you! *zzz*",
        "effect": "sleepy-sway",
        "sound": "*Yawn* Zzz... *snore*"
    },
    "hyperactive": {
        "emoji": "⚡",
        "bg": "#fff8dc",
        "message": "ZOOM ZOOM! Can't sit still! SO MUCH ENERGY!",
        "effect": "hyper-bounce",
        "sound": "WHEEE! *Zoom* *Zing*"
    },
    "philosophical": {
        "emoji": "🤔",
        "bg": "#f8f8ff",
        "message": "Hmm... what IS a cookie, really? Deep thoughts...",
        "effect": "thoughtful-nod",
        "sound": "Hmm... *strokes beard*"
    },
    "party": {
        "emoji": "🥳",
        "bg": "#fff0f5",
        "message": "PARTY TIME! Every visit is a celebration! 🎉",
        "effect": "party-dance",
        "sound": "🎵 *Party music* 🎵"
    },
    "mysterious": {
        "emoji": "🔮",
        "bg": "#e6e6fa",
        "message": "I see... interesting... your aura is magnificent...",
        "effect": "mystical-glow",
        "sound": "*Mysterious humming*"
    }
}

# 🎨 SUPER CUTE THEMES
gallery_themes = {
    "unicorn": {"emoji": "🦄", "name": "Unicorn Dreams", "bg": "linear-gradient(135deg, #ff9a9e 0%, #fecfef 50%, #fecfef 100%)"},
    "cupcake": {"emoji": "🧁", "name": "Cupcake Paradise", "bg": "linear-gradient(135deg, #ffecd2 0%, #fcb69f 100%)"},
    "mermaid": {"emoji": "🧜‍♀️", "name": "Mermaid Lagoon", "bg": "linear-gradient(135deg, #a8edea 0%, #fed6e3 100%)"},
    "cotton_candy": {"emoji": "🍭", "name": "Cotton Candy Clouds", "bg": "linear-gradient(135deg, #d299c2 0%, #fef9d7 100%)"},
    "rainbow": {"emoji": "🌈", "name": "Rainbow Magic", "bg": "linear-gradient(135deg, #ff9a9e 0%, #fecfef 25%, #a8edea 50%, #d299c2 75%, #fef9d7 100%)"},
    "starlight": {"emoji": "⭐", "name": "Starlight Wonder", "bg": "linear-gradient(135deg, #667eea 0%, #764ba2 100%)"},
    "fairy": {"emoji": "🧚‍♀️", "name": "Fairy Garden", "bg": "linear-gradient(135deg, #c3cfe2 0%, #c3cfe2 100%)"},
    "butterfly": {"emoji": "🦋", "name": "Butterfly Meadow", "bg": "linear-gradient(135deg, #ffecd2 0%, #fcb69f 100%)"}
}

# Cookie management logic with EXTRA CUTENESS
headers = []
action = params.get('action', '')
visitor_name = params.get('name', cookies.get('visitor_name', ''))
favorite_emoji = params.get('emoji', cookies.get('favorite_emoji', '🌸'))
personality = params.get('personality', cookies.get('personality_mode', 'bubbly'))
theme = params.get('theme', cookies.get('gallery_theme', 'unicorn'))
favorite_color = params.get('color', cookies.get('favorite_color', '#ff69b4'))
magic_level = int(cookies.get('magic_level', '1'))
visit_count = int(cookies.get('visit_count', '0'))
last_visit = cookies.get('last_visit', '')
reset_message = ""

# Increment visit count and magic level!
visit_count += 1
if visit_count > 1:
    magic_level = min(magic_level + 1, 100)  # Cap at 100 for maximum magic!

# Get current personality info
current_personality = personality_modes.get(personality, personality_modes['bubbly'])
current_theme = gallery_themes.get(theme, gallery_themes['unicorn'])

# Handle different actions with MAXIMUM CUTENESS
if action == 'reset':
    # Sad but cute reset message
    cookie_names = ['visitor_name', 'favorite_emoji', 'personality_mode', 'gallery_theme',
                   'favorite_color', 'magic_level', 'visit_count', 'last_visit', 'cookie_happiness']
    for name in cookie_names:
        headers.append(f"Set-Cookie: {name}=; Path=/; Max-Age=0")


    # Reset everything to defaults
    visitor_name = ''
    favorite_emoji = '🌸'
    personality = 'bubbly'
    theme = 'unicorn'
    favorite_color = '#ff69b4'
    magic_level = 1
    visit_count = 0
    reset_message = "🥺 Aww, cookies reset! But don't worry, we can make NEW magical memories! ✨"

elif action == 'save' or any([visitor_name, personality != 'bubbly', theme != 'unicorn']):
    # Save all the magical preferences!
    expire_date = (datetime.now() + timedelta(days=365)).strftime("%a, %d %b %Y %H:%M:%S GMT")  # 1 year of magic!

    if visitor_name:
        headers.append(f"Set-Cookie: visitor_name={urllib.parse.quote(visitor_name)}; Path=/; Expires={expire_date}")
    headers.append(f"Set-Cookie: favorite_emoji={favorite_emoji}; Path=/; Expires={expire_date}")
    headers.append(f"Set-Cookie: personality_mode={personality}; Path=/; Expires={expire_date}")
    headers.append(f"Set-Cookie: gallery_theme={theme}; Path=/; Expires={expire_date}")
    headers.append(f"Set-Cookie: favorite_color={urllib.parse.quote(favorite_color)}; Path=/; Expires={expire_date}")
    headers.append(f"Set-Cookie: magic_level={magic_level}; Path=/; Expires={expire_date}")
    headers.append(f"Set-Cookie: visit_count={visit_count}; Path=/; Expires={expire_date}")
    headers.append(f"Set-Cookie: last_visit={datetime.now().isoformat()}; Path=/; Expires={expire_date}")

    # Calculate cookie happiness level!
    happiness_score = min(visit_count * 10 + magic_level, 1000)
    headers.append(f"Set-Cookie: cookie_happiness={happiness_score}; Path=/; Expires={expire_date}")

# Generate super fun facts!
def get_magic_status(level):
    if level <= 10:
        return "✨ Sparkle Apprentice"
    elif level <= 25:
        return "🌟 Twinkle Master"
    elif level <= 50:
        return "🦄 Unicorn Whisperer"
    elif level <= 75:
        return "🧚‍♀️ Fairy Godperson"
    else:
        return "🔮 Supreme Magic Being"

def get_fun_fact():
    facts = [
        "🍪 Did you know? This cookie contains 0 calories but 100% pure happiness!",
        "🌟 Fun fact: Unicorns personally approve of your excellent taste!",
        "🎭 Science says: Your smile increased the server's processing speed by 200%!",
        "🦋 Breaking discovery: Butterflies want to be friends with you!",
        "🍓 Latest research: You're 47% more magical than the average human!",
        "🌈 Weather update: Your presence caused spontaneous rainbow formation!",
        "⭐ Astronomical news: Stars rearranged themselves to spell 'YOU'RE AWESOME'!",
        "🎪 Circus report: Even the clowns are jealous of how fun you are!"
    ]
    return random.choice(facts)

# Choose welcome message
if visit_count > 1 and visitor_name:
    welcome_message = random.choice(funny_welcome_messages)
else:
    welcome_message = random.choice(first_time_messages)

# Set headers
headers.append("Content-Type: text/html; charset=utf-8")

# Send all headers
for header in headers:
    sys.stdout.write(header + "\r\n")
sys.stdout.write("\r\n")

# 🎨 THE MOST ADORABLE HTML EVER CREATED! 🎨
html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🍪 Magical Cookie Kingdom - Where Dreams Come True! ✨</title>
    <script src="/scripts/theme.js"></script>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Comic+Neue:wght@400;700&display=swap');

        body {{
            font-family: 'Comic Neue', 'Comic Sans MS', cursive, sans-serif;
            background: {current_theme['bg']};
            margin: 0;
            padding: 20px;
            min-height: 100vh;
            position: relative;
            overflow-x: hidden;
            animation: gentle-pulse 4s ease-in-out infinite;
        }}

        @keyframes gentle-pulse {{
            0%, 100% {{ filter: brightness(1); }}
            50% {{ filter: brightness(1.05); }}
        }}

        .magic-container {{
            max-width: 1300px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 30px;
            padding: 40px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
            position: relative;
            border: 3px solid {favorite_color};
            animation: container-glow 3s ease-in-out infinite;
        }}

        @keyframes container-glow {{
            0%, 100% {{ box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1); }}
            50% {{ box-shadow: 0 25px 50px rgba(255, 105, 180, 0.3); }}
        }}

        .header-magic {{
            text-align: center;
            margin-bottom: 30px;
            position: relative;
        }}

        .mega-emoji {{
            font-size: 6rem;
            margin: 20px 0;
            animation: {current_personality['effect']} 3s ease-in-out infinite;
            text-shadow: 0 0 20px rgba(255, 105, 180, 0.5);
        }}

        .magic-title {{
            background: linear-gradient(135deg, #ff6b6b, #4ecdc4, #45b7d1, #96ceb4, #feca57);
            background-size: 300% 300%;
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            animation: rainbow-text 3s ease-in-out infinite;
            font-size: 2.5rem;
            font-weight: bold;
            margin: 15px 0;
        }}

        @keyframes rainbow-text {{
            0%, 100% {{ background-position: 0% 50%; }}
            50% {{ background-position: 100% 50%; }}
        }}

        .welcome-explosion {{
            background: linear-gradient(135deg, rgba(255, 255, 255, 0.9), rgba(255, 182, 193, 0.8));
            border: 3px solid #ff69b4;
            border-radius: 25px;
            padding: 25px;
            margin: 25px 0;
            position: relative;
            overflow: hidden;
            animation: welcome-bounce 2s ease-in-out infinite;
        }}

        @keyframes welcome-bounce {{
            0%, 100% {{ transform: translateY(0) scale(1); }}
            50% {{ transform: translateY(-5px) scale(1.02); }}
        }}

        .personality-showcase {{
            background: {current_personality['bg']};
            border-radius: 20px;
            padding: 20px;
            margin: 20px 0;
            border: 2px dashed #ff69b4;
            text-align: center;
        }}

        .stats-wonderland {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin: 25px 0;
        }}

        .stat-bubble {{
            background: linear-gradient(135deg, #fff 0%, #f0f8ff 100%);
            border-radius: 20px;
            padding: 20px;
            text-align: center;
            border: 2px solid {favorite_color};
            transition: all 0.3s ease;
            animation: stat-float 4s ease-in-out infinite;
        }}

        .stat-bubble:hover {{
            transform: scale(1.1) rotate(2deg);
            box-shadow: 0 10px 25px rgba(255, 105, 180, 0.4);
        }}

        @keyframes stat-float {{
            0%, 100% {{ transform: translateY(0); }}
            50% {{ transform: translateY(-8px); }}
        }}

        .stat-number {{
            font-size: 2.5rem;
            font-weight: bold;
            background: linear-gradient(135deg, #ff6b6b, #4ecdc4);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            margin-bottom: 8px;
        }}

        .form-wonderland {{
            background: linear-gradient(135deg, #ffeef5 0%, #f0f8ff 100%);
            border-radius: 25px;
            padding: 30px;
            margin: 25px 0;
            border: 3px solid #ffb6c1;
        }}

        .form-group {{
            margin: 20px 0;
        }}

        .cute-label {{
            display: block;
            margin-bottom: 10px;
            font-weight: bold;
            color: #ff1493;
            font-size: 1.1rem;
        }}

        .magic-input, .magic-select {{
            width: 100%;
            padding: 15px;
            border: 3px solid #ffb6c1;
            border-radius: 15px;
            font-size: 16px;
            font-family: inherit;
            background: rgba(255, 255, 255, 0.9);
            box-sizing: border-box;
            transition: all 0.3s ease;
        }}

        .magic-input:focus, .magic-select:focus {{
            outline: none;
            border-color: #ff69b4;
            box-shadow: 0 0 15px rgba(255, 105, 180, 0.5);
            transform: scale(1.02);
        }}

        .personality-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }}

        .personality-card {{
            background: white;
            border: 3px solid #ffb6c1;
            border-radius: 15px;
            padding: 15px;
            text-align: center;
            cursor: pointer;
            transition: all 0.3s ease;
            position: relative;
        }}

        .personality-card:hover {{
            transform: scale(1.1) rotate(-2deg);
            border-color: #ff69b4;
            box-shadow: 0 10px 20px rgba(255, 105, 180, 0.3);
        }}

        .personality-card.selected {{
            background: linear-gradient(135deg, #ff69b4, #ff1493);
            color: white;
            border-color: #ff1493;
            animation: selected-glow 2s ease-in-out infinite;
        }}

        @keyframes selected-glow {{
            0%, 100% {{ box-shadow: 0 0 15px rgba(255, 105, 180, 0.5); }}
            50% {{ box-shadow: 0 0 25px rgba(255, 105, 180, 0.8); }}
        }}

        .theme-showcase {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
            gap: 12px;
            margin: 20px 0;
        }}

        .theme-preview {{
            height: 80px;
            border-radius: 12px;
            cursor: pointer;
            border: 3px solid transparent;
            transition: all 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 2rem;
            position: relative;
        }}

        .theme-preview:hover {{
            transform: scale(1.1);
            border-color: #ff69b4;
        }}

        .theme-preview.selected {{
            border-color: #ff1493;
            box-shadow: 0 0 20px rgba(255, 105, 180, 0.6);
        }}

        .magic-button {{
            background: linear-gradient(135deg, #ff6b6b 0%, #4ecdc4 50%, #45b7d1 100%);
            background-size: 200% 200%;
            color: white;
            border: none;
            padding: 15px 30px;
            border-radius: 25px;
            font-size: 18px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            animation: button-rainbow 3s ease-in-out infinite;
            margin: 10px;
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.2);
        }}

        .magic-button:hover {{
            transform: translateY(-3px) scale(1.05);
            box-shadow: 0 12px 30px rgba(255, 105, 180, 0.4);
            animation-duration: 1s;
        }}

        @keyframes button-rainbow {{
            0%, 100% {{ background-position: 0% 50%; }}
            50% {{ background-position: 100% 50%; }}
        }}

        .fun-fact-box {{
            background: linear-gradient(135deg, #fff9c4 0%, #f093fb 100%);
            border: 3px dashed #ff69b4;
            border-radius: 20px;
            padding: 20px;
            margin: 20px 0;
            text-align: center;
            animation: fact-wiggle 4s ease-in-out infinite;
        }}

        @keyframes fact-wiggle {{
            0%, 100% {{ transform: rotate(0deg); }}
            25% {{ transform: rotate(1deg); }}
            75% {{ transform: rotate(-1deg); }}
        }}

        .floating-hearts {{
            position: absolute;
            font-size: 2rem;
            opacity: 0.6;
            animation: float-around 8s ease-in-out infinite;
            pointer-events: none;
        }}

        @keyframes float-around {{
            0%, 100% {{ transform: translateY(0) rotate(0deg); }}
            25% {{ transform: translateY(-20px) rotate(90deg); }}
            50% {{ transform: translateY(-10px) rotate(180deg); }}
            75% {{ transform: translateY(-30px) rotate(270deg); }}
        }}

        /* Personality-specific animations */
        @keyframes bubble-float {{
            0%, 100% {{ transform: translateY(0) scale(1); }}
            50% {{ transform: translateY(-15px) scale(1.1); }}
        }}

        @keyframes giggle-shake {{
            0%, 100% {{ transform: rotate(0deg); }}
            25% {{ transform: rotate(2deg); }}
            75% {{ transform: rotate(-2deg); }}
        }}

        @keyframes dramatic-flair {{
            0%, 100% {{ transform: scale(1) rotate(0deg); }}
            25% {{ transform: scale(1.1) rotate(5deg); }}
            75% {{ transform: scale(1.1) rotate(-5deg); }}
        }}

        @keyframes sleepy-sway {{
            0%, 100% {{ transform: translateX(0); }}
            50% {{ transform: translateX(10px); }}
        }}

        @keyframes hyper-bounce {{
            0%, 100% {{ transform: translateY(0) scale(1); }}
            25% {{ transform: translateY(-20px) scale(1.1); }}
            50% {{ transform: translateY(-10px) scale(0.9); }}
            75% {{ transform: translateY(-25px) scale(1.2); }}
        }}

        @keyframes thoughtful-nod {{
            0%, 100% {{ transform: translateY(0); }}
            50% {{ transform: translateY(-8px); }}
        }}

        @keyframes party-dance {{
            0%, 100% {{ transform: rotate(0deg) scale(1); }}
            25% {{ transform: rotate(10deg) scale(1.1); }}
            50% {{ transform: rotate(0deg) scale(1.2); }}
            75% {{ transform: rotate(-10deg) scale(1.1); }}
        }}

        @keyframes mystical-glow {{
            0%, 100% {{ filter: hue-rotate(0deg) brightness(1); }}
            50% {{ filter: hue-rotate(180deg) brightness(1.2); }}
        }}

        .cookie-display {{
            background: linear-gradient(135deg, #ffeaa7 0%, #fab1a0 100%);
            border: 3px solid #fdcb6e;
            border-radius: 15px;
            padding: 20px;
            margin: 20px 0;
            font-family: 'Courier New', monospace;
            font-size: 14px;
            max-height: 200px;
            overflow-y: auto;
        }}
    </style>
</head>
<body>
    <!-- Floating decorative elements -->
    <div class="floating-hearts" style="top: 10%; left: 10%; animation-delay: 0s;">💖</div>
    <div class="floating-hearts" style="top: 20%; right: 15%; animation-delay: 1s;">✨</div>
    <div class="floating-hearts" style="bottom: 25%; left: 20%; animation-delay: 2s;">🦄</div>
    <div class="floating-hearts" style="bottom: 15%; right: 25%; animation-delay: 3s;">{favorite_emoji}</div>
    <div class="floating-hearts" style="top: 50%; left: 5%; animation-delay: 4s;">🌈</div>

    <div class="magic-container">
        <!-- Magical Header -->
        <div class="header-magic">
            <div class="mega-emoji">{current_theme['emoji']} 🍪 {current_personality['emoji']}</div>
            <h1 class="magic-title">Magical Cookie Kingdom</h1>
            <p style="font-size: 1.2rem; color: #666; margin: 0;">
                Where every click creates magic and every cookie tells a story! ✨
            </p>
        </div>

        <!-- Welcome Explosion -->
        <div class="welcome-explosion">
            <h2 style="color: #ff1493; margin: 0 0 15px 0; font-size: 1.8rem;">
                {welcome_message}
            </h2>
            <div style="font-size: 1.1rem; color: #555;">
                {f"<strong style='color: #ff1493;'>{visitor_name}</strong>, you magnificent human! " if visitor_name else ""}
                {current_personality['message']}
            </div>
            <div style="margin-top: 15px; font-style: italic; color: #888;">
                🔊 Cookie says: "{current_personality['sound']}"
            </div>
        </div>


        <!-- Fun Fact Box -->
        <div class="fun-fact-box">
            <h3 style="color: #ff1493; margin: 0 0 10px 0;">🎯 Super Fun Fact!</h3>
            <p style="margin: 0; font-size: 1.1rem; font-weight: bold;">
                {get_fun_fact()}
            </p>
        </div>

        <!-- Magical Customization Form -->
        <form method="POST" action="">
            <input type="hidden" name="action" value="save">

            <div class="form-wonderland">
                <h3 style="color: #ff1493; margin: 0 0 20px 0; text-align: center;">
                    🌟 Customize Your Magical Experience! 🌟
                </h3>

                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 25px;">
                    <!-- Left Column -->
                    <div>
                        <div class="form-group">
                            <label class="cute-label" for="name">
                                ✨ What magical name should we call you? (Optional but SO much more fun!)
                            </label>
                            <input type="text" id="name" name="name" class="magic-input"
                                   value="{visitor_name}" placeholder="Your amazing name...">
                        </div>

                        <div class="form-group">
                            <label class="cute-label" for="emoji">🎭 Pick your spirit emoji!</label>
                            <select id="emoji" name="emoji" class="magic-select">
                                <option value="🌸" {'selected' if favorite_emoji == '🌸' else ''}>🌸 Cherry Blossom Magic</option>
                                <option value="🦄" {'selected' if favorite_emoji == '🦄' else ''}>🦄 Unicorn Power</option>
                                <option value="✨" {'selected' if favorite_emoji == '✨' else ''}>✨ Sparkle Goddess</option>
                                <option value="🌙" {'selected' if favorite_emoji == '🌙' else ''}>🌙 Moon Child</option>
                                <option value="🦋" {'selected' if favorite_emoji == '🦋' else ''}>🦋 Butterfly Dreams</option>
                                <option value="🌈" {'selected' if favorite_emoji == '🌈' else ''}>🌈 Rainbow Spirit</option>
                                <option value="💎" {'selected' if favorite_emoji == '💎' else ''}>💎 Diamond Soul</option>
                                <option value="🍓" {'selected' if favorite_emoji == '🍓' else ''}>🍓 Sweet Berry</option>
                                <option value="🎭" {'selected' if favorite_emoji == '🎭' else ''}>🎭 Drama Queen/King</option>
                                <option value="🔮" {'selected' if favorite_emoji == '🔮' else ''}>🔮 Crystal Mystic</option>
                            </select>
                        </div>

                        <div class="form-group">
                            <label class="cute-label" for="color">🎨 Your magical color aura:</label>
                            <input type="color" id="color" name="color" class="magic-input"
                                   value="{favorite_color}" style="height: 60px; cursor: pointer;">
                        </div>
                    </div>

                    <!-- Right Column -->
                    <div>
                        <div class="form-group">
                            <label class="cute-label">🎭 Choose your personality mode!</label>
                            <div class="personality-grid">"""

# Add personality options
for mode_key, mode_info in personality_modes.items():
    selected_class = 'selected' if personality == mode_key else ''
    html_content += f'''
                                <div class="personality-card {selected_class}" onclick="selectPersonality('{mode_key}')">
                                    <div style="font-size: 2rem; margin-bottom: 5px;">{mode_info['emoji']}</div>
                                    <div style="font-weight: bold; font-size: 0.9rem;">{mode_key.title()}</div>
                                    <div style="font-size: 0.8rem; margin-top: 5px; opacity: 0.8;">{mode_info['sound']}</div>
                                </div>'''

html_content += f"""
                            </div>
                            <input type="hidden" id="personality" name="personality" value="{personality}">
                        </div>

                        <div class="form-group">
                            <label class="cute-label">🎨 Gallery theme selection!</label>
                            <div class="theme-showcase">"""

# Add theme options
for theme_key, theme_info in gallery_themes.items():
    selected_class = 'selected' if theme == theme_key else ''
    html_content += f'''
                                <div class="theme-preview {selected_class}"
                                     style="background: {theme_info['bg']};"
                                     onclick="selectTheme('{theme_key}')"
                                     title="{theme_info['name']}">
                                    {theme_info['emoji']}
                                </div>'''

html_content += f"""
                            </div>
                            <input type="hidden" id="theme" name="theme" value="{theme}">
                        </div>
                    </div>
                </div>

                <div style="text-align: center; margin-top: 30px;">
                    <button type="submit" class="magic-button">
                        ✨🍪 Save My Magical Preferences! 🍪✨
                    </button>
                </div>
            </div>
        </form>

        <!-- Technical Cookie Details (for the curious minds!) -->
        <div style="background: linear-gradient(135deg, #e8f5e8 0%, #f0f8ff 100%); border-radius: 20px; padding: 25px; margin: 25px 0;">
            <h3 style="color: #ff1493; margin: 0 0 15px 0; text-align: center;">
                🔧 Technical Cookie Magic Details 🔧
            </h3>

            <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin: 20px 0;">
                <div style="background: white; padding: 15px; border-radius: 15px; border: 2px solid #4ecdc4;">
                    <h4 style="color: #4ecdc4; margin: 0 0 10px 0;">📊 Your Cookie Stats</h4>
                    <p style="margin: 5px 0;"><strong>🍪 Cookies Set:</strong> {len([c for c in cookies.keys() if not c.startswith('_')])}</p>
                    <p style="margin: 5px 0;"><strong>⏰ Session Start:</strong> {datetime.now().strftime('%H:%M:%S')}</p>
                    <p style="margin: 5px 0;"><strong>🎯 Magic Score:</strong> {cookies.get('cookie_happiness', '100')}</p>
                    <p style="margin: 5px 0;"><strong>🌟 Status:</strong> {'Magical VIP' if visit_count > 5 else 'Honored Guest'}</p>
                </div>

                <div style="background: white; padding: 15px; border-radius: 15px; border: 2px solid #ff6b6b;">
                    <h4 style="color: #ff6b6b; margin: 0 0 10px 0;">🎨 Current Settings</h4>
                    <p style="margin: 5px 0;"><strong>👤 Name:</strong> {visitor_name or 'Anonymous Cutie'}</p>
                    <p style="margin: 5px 0;"><strong>🎭 Personality:</strong> {personality.title()}</p>
                    <p style="margin: 5px 0;"><strong>🌈 Theme:</strong> {current_theme['name']}</p>
                    <p style="margin: 5px 0;"><strong>✨ Emoji:</strong> {favorite_emoji}</p>
                </div>
            </div>

            <div class="cookie-display">
                <h4 style="color: #fdcb6e; margin: 0 0 10px 0;">🔍 Raw Cookie Data (for the tech-savvy!)</h4>
                <div style="max-height: 150px; overflow-y: auto; font-size: 12px; line-height: 1.4;">
                    {cookie_header if cookie_header else 'No cookies yet - time to bake some digital magic! 🍪✨'}
                </div>
            </div>
        </div>

        <!-- Action Buttons -->
        <div style="text-align: center; margin: 30px 0;">
            <a href="/web/playground.html" class="magic-button" style="text-decoration: none; background: linear-gradient(135deg, #6c5ce7 0%, #a29bfe 100%);">
                🔙 Back to the Playground
            </a>

            <a href="/" class="magic-button" style="text-decoration: none; background: linear-gradient(135deg, #00b894 0%, #00cec9 100%);">
                🏠 Gallery Home
            </a>

            <button onclick="generateMagicalReport()" class="magic-button" style="background: linear-gradient(135deg, #fdcb6e 0%, #e17055 100%);">
                📊 Generate Magic Report
            </button>

            <a href="?action=reset" class="magic-button" style="text-decoration: none; background: linear-gradient(135deg, #fd79a8 0%, #e84393 100%);"
               onclick="return confirm('🥺 Are you ABSOLUTELY sure you want to reset all your magical preferences? This will make the cookies very sad! 💔')">
               🧹 Reset All Magic
            </a>
        </div>

        <!-- Final Magic Message -->
        <div style="background: linear-gradient(135deg, #74b9ff 0%, #0984e3 100%); color: white; border-radius: 20px; padding: 25px; text-align: center; margin: 25px 0;">
            <h3 style="margin: 0 0 15px 0;">🌟 Thank You for Visiting Our Magical Cookie Kingdom! 🌟</h3>
            <p style="margin: 0; font-size: 1.1rem; line-height: 1.6;">
                Every time you visit, you make our server a little bit happier!
                Your preferences are safely stored in magical cookie jars,
                and they'll remember you for a whole year of adventures! ✨🍪💖
            </p>
            <div style="margin-top: 15px; font-size: 1.5rem;">
                {current_theme['emoji']} {favorite_emoji} {current_personality['emoji']}
            </div>
        </div>
    </div>

    <script>
        // Magical JavaScript functions! ✨

        function selectPersonality(mode) {{
            document.getElementById('personality').value = mode;

            // Update visual selection
            document.querySelectorAll('.personality-card').forEach(card => {{
                card.classList.remove('selected');
            }});
            event.currentTarget.classList.add('selected');

            // Add some magic feedback
            showMagicalNotification(`✨ Personality changed to ${{mode}}! You're feeling magical! ✨`);
        }}

        function selectTheme(themeKey) {{
            document.getElementById('theme').value = themeKey;

            // Update visual selection
            document.querySelectorAll('.theme-preview').forEach(preview => {{
                preview.classList.remove('selected');
            }});
            event.currentTarget.classList.add('selected');

            showMagicalNotification(`🎨 Theme updated! Your world is now more beautiful! 🌟`);
        }}

        function showMagicalNotification(message) {{
            const notification = document.createElement('div');
            notification.style.cssText = `
                position: fixed;
                top: 20px;
                right: 20px;
                background: linear-gradient(135deg, #ff6b6b, #4ecdc4);
                color: white;
                padding: 15px 25px;
                border-radius: 25px;
                z-index: 10000;
                animation: magical-slide-in 0.5s ease-out;
                box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
                font-weight: bold;
                max-width: 300px;
            `;

            notification.textContent = message;
            document.body.appendChild(notification);

            setTimeout(() => {{
                notification.style.animation = 'magical-slide-out 0.5s ease-in forwards';
                setTimeout(() => notification.remove(), 500);
            }}, 3000);
        }}

        function generateMagicalReport() {{
            const report = `
🌟 MAGICAL COOKIE REPORT 🌟

👤 Visitor Information:
   • Name: {visitor_name or 'Anonymous Magical Being'}
   • Spirit Emoji: {favorite_emoji}
   • Magic Level: {magic_level}/100 ({get_magic_status(magic_level)})
   • Personality Mode: {personality.title()} {current_personality['emoji']}

🎨 Customization Details:
   • Gallery Theme: {current_theme['name']} {current_theme['emoji']}
   • Favorite Color: {favorite_color}
   • Cookie Happiness: {cookies.get('cookie_happiness', '100')}/1000

📊 Visit Statistics:
   • Total Visits: {visit_count}
   • Session Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
   • Last Visit: {last_visit or 'First time!'}
   • Active Cookies: {len(cookies)}

🎯 Fun Facts:
   • You've made our server {visit_count * 10}% happier!
   • Your magical aura extends {magic_level * 2} pixels!
   • Unicorns approve of your choices by {min(magic_level * 3, 100)}%!

✨ Thank you for being absolutely AMAZING! ✨
            `;

            alert(report);
        }}

        // Add magical sparkle effects
        function createMagicalSparkle() {{
            const sparkle = document.createElement('div');
            sparkle.innerHTML = '✨';
            sparkle.style.cssText = `
                position: fixed;
                left: ${{Math.random() * window.innerWidth}}px;
                top: ${{Math.random() * window.innerHeight}}px;
                font-size: ${{15 + Math.random() * 15}}px;
                pointer-events: none;
                z-index: 9999;
                animation: sparkle-magic 4s ease-out forwards;
                color: {favorite_color};
            `;

            document.body.appendChild(sparkle);
            setTimeout(() => sparkle.remove(), 4000);
        }}

        // Create sparkles every few seconds
        setInterval(createMagicalSparkle, 2000);

        // Add magical sound effects (if supported)
        function playMagicalSound() {{
            if ('speechSynthesis' in window && {str(visit_count == 1).lower()}) {{
                const msg = new SpeechSynthesisUtterance("Welcome to the magical cookie kingdom!");
                msg.rate = 0.8;
                msg.pitch = 1.3;
                msg.volume = 0.7;
                speechSynthesis.speak(msg);
            }}
        }}

        // Initialize magic
        document.addEventListener('DOMContentLoaded', function() {{
            setTimeout(playMagicalSound, 1500);

            // Add hover effects to stats
            document.querySelectorAll('.stat-bubble').forEach(bubble => {{
                bubble.addEventListener('mouseenter', function() {{
                    this.style.transform = 'scale(1.1) rotate(2deg)';
                }});

                bubble.addEventListener('mouseleave', function() {{
                    this.style.transform = '';
                }});
            }});

            // Show welcome message for first-time visitors
            if ({str(visit_count == 1).lower()}) {{
                setTimeout(() => {{
                    showMagicalNotification('🎉 Welcome to the most magical cookie experience ever! 🍪✨');
                }}, 2000);
            }}
        }});

        // Add CSS for magical animations
        const magicalStyle = document.createElement('style');
        magicalStyle.textContent = `
            @keyframes magical-slide-in {{
                from {{ transform: translateX(100%); opacity: 0; }}
                to {{ transform: translateX(0); opacity: 1; }}
            }}

            @keyframes magical-slide-out {{
                from {{ transform: translateX(0); opacity: 1; }}
                to {{ transform: translateX(100%); opacity: 0; }}
            }}

            @keyframes sparkle-magic {{
                0% {{ opacity: 1; transform: scale(0) rotate(0deg); }}
                50% {{ opacity: 1; transform: scale(1) rotate(180deg); }}
                100% {{ opacity: 0; transform: scale(0) rotate(360deg); }}
            }}
        `;
        document.head.appendChild(magicalStyle);
    </script>
</body>
</html>
"""

# Output the magical HTML!
sys.stdout.write(html_content)