function getCookie(name) {
    const match = document.cookie.match(new RegExp('(?:^|; )' + name.replace(/([.$?*|{}()\[\]\\/+^])/g, '\\$1') + '=([^;]*)'));
    return match ? decodeURIComponent(match[1]) : null;
}

function setCookie(name, value, days) {
    const expires = new Date(Date.now() + days * 864e5).toUTCString();
    document.cookie = name + '=' + encodeURIComponent(value) + '; Path=/; Expires=' + expires;
}

function incrementVisitCount() {
    let count = parseInt(getCookie('visit_count') || '0', 10) + 1;
    setCookie('visit_count', count, 365);
    return count;
}

function ensureVisitBadge(count) {
    let badge = document.getElementById('visit-badge');
    if (!badge) {
        badge = document.createElement('div');
        badge.id = 'visit-badge';
        badge.style.cssText = 'position:fixed;bottom:10px;right:10px;' +
            'background:rgba(255,255,255,0.8);color:#ff1493;padding:4px 12px;' +
            'border-radius:12px;font-size:0.9rem;z-index:999;box-shadow:0 0 5px rgba(0,0,0,0.2)';
        badge.innerHTML = 'Visits: <span id="visit-count"></span>';
        document.body.appendChild(badge);
    }
    document.getElementById('visit-count').textContent = count;
}

function applyCookieTheme() {
    const theme = getCookie('gallery_theme') || 'pink';
    const personality = getCookie('personality_mode') || 'bubbly';
    const visitorName = getCookie('visitor_name');

	document.body.className = `theme-${theme} personality-${personality}`;

	if (visitorName) {
			showWelcomeMessage(`Welcome back, ${visitorName}! ✨`);
	}
}

// function applyCookiePreferences() {
// 	const theme = getCookie('gallery_theme') || 'pink';
// 	const personality = getCookie('personality_mode') || 'bubbly';
// 	const visitorName = getCookie('visitor_name');

// 	document.body.className = `theme-${theme} personality-${personality}`;

// 	if (visitorName) {
// 			showWelcomeMessage(`Welcome back, ${visitorName}! ✨`);
// 	}
// }
// document.addEventListener('DOMContentLoaded', applyCookiePreferences);


function showWelcomeMessage(msg) {
    const div = document.createElement('div');
    div.className = 'welcome-msg';
    div.textContent = msg;
    div.style.cssText = 'position:fixed;top:10px;left:50%;transform:translateX(-50%);background:#ff69b4;color:#fff;padding:10px 20px;border-radius:20px;z-index:1000;';
    document.body.appendChild(div);
    setTimeout(() => div.remove(), 3000);
}

document.addEventListener('DOMContentLoaded', () => {
    const count = incrementVisitCount();
    applyCookieTheme();
    ensureVisitBadge(count);
});
