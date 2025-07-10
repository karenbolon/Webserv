#!/usr/bin/env python3
"""
🫖☕ Interactive Coffee Shop → Teapot Script ☕🫖
First asks nicely if you want coffee, then delivers the 418 surprise!
"""

import sys
import os
import cgi
import urllib.parse
from datetime import datetime

# Get form data and query parameters
form = cgi.FieldStorage()
query_string = os.environ.get("QUERY_STRING", "")
want_coffee = form.getvalue("coffee") or "coffee=yes" in query_string

# Get current time
current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

if not want_coffee:
    # FIRST PAGE: Coffee invitation
    print("Content-Type: text/html; charset=utf-8\r")
    print("\r")

    print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>☕ Welcome to Our Coffee Shop! ☕</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Fredoka+One:wght@400&family=Comic+Neue:wght@400;700&display=swap');

        body {{
            background: linear-gradient(135deg, #6F4E37 0%, #8B4513 50%, #D2B48C 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            font-family: 'Comic Neue', cursive;
            margin: 0;
            padding: 20px;
            box-sizing: border-box;
        }}

        .coffee-shop {{
            background: rgba(255, 248, 220, 0.95);
            border-radius: 30px;
            padding: 50px;
            max-width: 800px;
            text-align: center;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.3);
            position: relative;
            border: 3px solid #8B4513;
        }}

        .coffee-shop::before {{
            content: '';
            position: absolute;
            top: -10px;
            left: -10px;
            right: -10px;
            bottom: -10px;
            background: linear-gradient(45deg, #D2691E, #CD853F, #DEB887, #F5DEB3);
            border-radius: 35px;
            z-index: -1;
            animation: coffee-glow 3s ease-in-out infinite;
        }}

        @keyframes coffee-glow {{
            0%, 100% {{ opacity: 0.7; }}
            50% {{ opacity: 1; }}
        }}

        .coffee-title {{
            font-size: 3rem;
            color: #8B4513;
            margin-bottom: 20px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.2);
            animation: coffee-bounce 2s ease-in-out infinite;
        }}

        @keyframes coffee-bounce {{
            0%, 100% {{ transform: translateY(0); }}
            50% {{ transform: translateY(-10px); }}
        }}

        .coffee-emoji {{
            font-size: 6rem;
            margin: 30px 0;
            animation: steam-rise 3s ease-in-out infinite;
        }}

        @keyframes steam-rise {{
            0%, 100% {{ transform: scale(1) rotate(0deg); }}
            25% {{ transform: scale(1.05) rotate(-2deg); }}
            50% {{ transform: scale(1.1) rotate(0deg); }}
            75% {{ transform: scale(1.05) rotate(2deg); }}
        }}

        .welcome-message {{
            font-size: 1.5rem;
            color: #654321;
            margin: 30px 0;
            line-height: 1.6;
        }}

        .coffee-question {{
            background: linear-gradient(135deg, #F5DEB3 0%, #DEB887 100%);
            border: 3px dashed #8B4513;
            border-radius: 20px;
            padding: 30px;
            margin: 30px 0;
            font-size: 1.3rem;
            color: #8B4513;
            font-weight: bold;
        }}

        .button-container {{
            margin: 40px 0;
            display: flex;
            gap: 20px;
            justify-content: center;
            flex-wrap: wrap;
        }}

        .coffee-button {{
            background: linear-gradient(135deg, #8B4513 0%, #A0522D 100%);
            color: white;
            border: none;
            padding: 20px 40px;
            border-radius: 50px;
            font-size: 1.2rem;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            text-decoration: none;
            display: inline-flex;
            align-items: center;
            gap: 10px;
            box-shadow: 0 8px 20px rgba(139, 69, 19, 0.3);
        }}

        .coffee-button:hover {{
            transform: translateY(-5px) scale(1.05);
            box-shadow: 0 15px 30px rgba(139, 69, 19, 0.5);
        }}

        .coffee-button.no {{
            background: linear-gradient(135deg, #696969 0%, #808080 100%);
        }}

        .coffee-button.no:hover {{
            box-shadow: 0 15px 30px rgba(105, 105, 105, 0.5);
        }}

        .floating-beans {{
            position: absolute;
            font-size: 2rem;
            opacity: 0.3;
            animation: float-around 8s ease-in-out infinite;
            pointer-events: none;
        }}

        @keyframes float-around {{
            0%, 100% {{ transform: translateY(0) rotate(0deg); }}
            25% {{ transform: translateY(-20px) rotate(90deg); }}
            50% {{ transform: translateY(-10px) rotate(180deg); }}
            75% {{ transform: translateY(-25px) rotate(270deg); }}
        }}

        .menu-hint {{
            background: rgba(139, 69, 19, 0.1);
            border-radius: 15px;
            padding: 20px;
            margin: 25px 0;
            font-style: italic;
            color: #8B4513;
        }}

        .timestamp {{
            color: #8B4513;
            font-size: 0.9rem;
            margin-top: 30px;
            opacity: 0.8;
        }}
    </style>
</head>
<body>
    <!-- Floating coffee beans -->
    <div class="floating-beans" style="top: 10%; left: 10%; animation-delay: 0s;">☕</div>
    <div class="floating-beans" style="top: 20%; right: 15%; animation-delay: 2s;">☕</div>
    <div class="floating-beans" style="bottom: 25%; left: 20%; animation-delay: 4s;">☕</div>
    <div class="floating-beans" style="bottom: 15%; right: 25%; animation-delay: 6s;">☕</div>

    <div class="coffee-shop">
        <h1 class="coffee-title">☕ Welcome to Our Coffee Shop! ☕</h1>

        <div class="coffee-emoji">☕</div>

        <div class="welcome-message">
            <p>🌟 <strong>Welcome, dear coffee lover!</strong> 🌟</p>
            <p>We're so excited to serve you today!</p>
        </div>

        <div class="coffee-question">
            <p>☕ Would you like to order some fresh, delicious coffee? ☕</p>
            <p style="margin-top: 15px; font-size: 1.1rem;">
                We have the finest beans, perfectly roasted, ready to brew! ✨
            </p>
        </div>

        <div class="menu-hint">
            <p>🍃 Our specialties include: Espresso, Cappuccino, Latte, Americano, and more!</p>
            <p>💫 Each cup is brewed with love and the finest equipment!</p>
        </div>

        <div class="button-container">
            <a href="?coffee=yes" class="coffee-button">
                ☕ Yes Please! I'd Love Coffee! ☕
            </a>

            <a href="/" class="coffee-button no">
                🚫 No Thanks, Maybe Later
            </a>
        </div>

        <div class="timestamp">
            Visit time: {current_time}<br>
            ☕ Ready to serve since 1998! ☕
        </div>
    </div>

    <script>
        // Add some interactive coffee magic
        document.addEventListener('DOMContentLoaded', function() {{
            // Add coffee aroma effect
            function createAroma() {{
                const aroma = document.createElement('div');
                aroma.innerHTML = '☁️';
                aroma.style.cssText = `
                    position: fixed;
                    left: ${{Math.random() * window.innerWidth}}px;
                    top: ${{window.innerHeight}}px;
                    font-size: ${{15 + Math.random() * 10}}px;
                    pointer-events: none;
                    z-index: 1000;
                    animation: aroma-rise 4s ease-out forwards;
                    opacity: 0.6;
                `;

                document.body.appendChild(aroma);
                setTimeout(() => aroma.remove(), 4000);
            }}

            // Create aroma every 2 seconds
            setInterval(createAroma, 2000);

            // Add hover sound effect (visual feedback)
            document.querySelectorAll('.coffee-button').forEach(button => {{
                button.addEventListener('mouseenter', function() {{
                    this.style.filter = 'brightness(1.1)';
                }});

                button.addEventListener('mouseleave', function() {{
                    this.style.filter = 'brightness(1)';
                }});
            }});
        }});

        // Add CSS for aroma animation
        const style = document.createElement('style');
        style.textContent = `
            @keyframes aroma-rise {{
                0% {{ transform: translateY(0) rotate(0deg); opacity: 0.6; }}
                50% {{ transform: translateY(-150px) rotate(180deg); opacity: 0.4; }}
                100% {{ transform: translateY(-300px) rotate(360deg); opacity: 0; }}
            }}
        `;
        document.head.appendChild(style);
    </script>
</body>
</html>
""")
    sys.stdout.flush()  # ✅ Flush exception error page

    # ✅ Ensure the 418 error script exits cleanly
    sys.exit(0)

    # ✅ Ensure the script exits cleanly
    sys.exit(0)

else:
    # SECOND PAGE: 418 Teapot Error (when they want coffee)
    # Serve your existing 418.html error page with proper status
    print("Status: 418 I'm a teapot\r")
    print("Content-Type: text/html; charset=utf-8\r")
    print("X-Brew-Error: Cannot brew coffee with a teapot\r")
    print("X-Tea-Ready: Always available\r")
    print("Retry-After: Never (I'm permanently a teapot)\r")
    print("\r")

    # Try to load your existing 418.html error page
    error_418_path = "www/error/418.html"  # Adjust path as needed

    try:
        with open(error_418_path, 'r', encoding='utf-8') as f:
            error_content = f.read()
        print(error_content)
    except FileNotFoundError:
        # Fallback if 418.html doesn't exist
        print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>418 - I'm a Teapot</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            text-align: center;
            background: #f0f8ff;
            padding: 50px;
        }}
        .error-container {{
            max-width: 600px;
            margin: 0 auto;
            background: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }}
    </style>
</head>
<body>
    <div class="error-container">
        <h1>🫖 418 - I'm a Teapot!</h1>
        <p>Sorry! We can't brew coffee because we're actually a teapot!</p>
        <p>Coffee request denied at: {current_time}</p>
        <p><a href="javascript:history.back()">← Back to Coffee Shop</a></p>
        <p><a href="/">🏠 Go Home</a></p>
        <p><small>Error 418.html not found at: {error_418_path}</small></p>
    </div>
</body>
</html>
        """)
    except Exception as e:
        # Fallback for any other errors
        print(f"""
<!DOCTYPE html>
<html>
<head><title>418 - I'm a Teapot</title></head>
<body>
    <h1>🫖 418 - I'm a Teapot!</h1>
    <p>Error loading 418.html: {str(e)}</p>
    <p><a href="/">Go Home</a></p>
</body>
</html>
        """)
