(function () {
	var USERS_FILE = 'secrets/users.txt';
	var form = document.getElementById('loginForm');
	var container = document.getElementById('login-container');
	var resultEl = document.getElementById('login-result');
	var errorEl = document.getElementById('login-error');

	var LOGOUT_SECONDS = 60; // FOR TESTING (client-side countdown length)
	var SESSION_EXPIRES_KEY = 'session_expires_at';

	function parseUsers(text) {
		var lines = text.split(/\r?\n/);
		var users = {};
		lines.forEach(function (line) {
			line = line.trim();
			if (!line || line.charAt(0) === '#') return;
			var parts = line.split(':');
			if (parts.length >= 2) {
				var user = parts[0].trim();
				var pass = parts.slice(1).join(':').trim();
				users[user] = pass;
			}
		});
		return users;
	}

	function formatTime(s) {
		var mm = Math.floor(s / 60);
		var ss = s % 60;
		return (mm < 10 ? '0' + mm : mm) + ':' + (ss < 10 ? '0' + ss : ss);
	}

	function showLoggedIn(username) {
		displayLoggedIn(username);
	}

	function displayLoggedIn(username) {
		var containerLocal = document.getElementById('login-container');
		var resultLocal = document.getElementById('login-result');
		var errorLocal = document.getElementById('login-error');
		if (!resultLocal) return;
		if (containerLocal) containerLocal.style.display = 'none';
		if (errorLocal) errorLocal.style.display = 'none';
		resultLocal.style.display = 'block';
		resultLocal.innerHTML = '';

		var box = document.createElement('div');
		box.className = 'upload-container login-success';
		box.style.textAlign = 'center';
		box.innerHTML = '<h2>you are logged in as <span class="login-username">' + username + '</span></h2>';

		var countdown = document.createElement('div');
		countdown.id = 'logout-countdown';
		countdown.className = 'logout-countdown';
		countdown.style.marginTop = '18px';
		countdown.textContent = formatTime(LOGOUT_SECONDS);
		box.appendChild(countdown);

		var logoutBtn = document.createElement('button');
		logoutBtn.type = 'button';
		logoutBtn.className = 'main_button logout-button';
		logoutBtn.textContent = 'logout';
		logoutBtn.style.marginTop = '12px';
		box.appendChild(logoutBtn);

		resultLocal.appendChild(box);

		var remaining = LOGOUT_SECONDS;
		var timer;

		function performLogoutServerSide() {
			try {
				fetch('/logout', { credentials: 'include' }).catch(function () { });
			} catch (e) { }
			try { localStorage.removeItem(SESSION_EXPIRES_KEY); } catch (e) { }
		}

		function doLogout() {
			if (timer) clearInterval(timer);
			performLogoutServerSide();
			if (resultLocal) resultLocal.style.display = 'none';
			if (containerLocal) containerLocal.style.display = '';
			var u = document.getElementById('username'); if (u) u.value = '';
			var p = document.getElementById('password'); if (p) p.value = '';
		}

		logoutBtn.addEventListener('click', function () {
			doLogout();
		});

		try {
			var expires = parseInt(localStorage.getItem(SESSION_EXPIRES_KEY), 10);
			if (!expires || isNaN(expires)) {
				expires = Date.now() + LOGOUT_SECONDS * 1000;
				localStorage.setItem(SESSION_EXPIRES_KEY, String(expires));
			}
			remaining = Math.max(0, Math.ceil((expires - Date.now()) / 1000));
		} catch (e) { remaining = LOGOUT_SECONDS; }
		countdown.textContent = formatTime(remaining);

		timer = setInterval(function () {
			remaining = Math.max(0, Math.ceil((parseInt(localStorage.getItem(SESSION_EXPIRES_KEY), 10) - Date.now()) / 1000));
			countdown.textContent = formatTime(remaining);
			if (remaining <= 0) {
				doLogout();
			}
		}, 1000);
	}

	if (form) {
		form.addEventListener('submit', function (e) {
			e.preventDefault();
			errorEl.style.display = 'none';
			var username = document.getElementById('username').value.trim();
			var password = document.getElementById('password').value;

			var loginUrl = '/login?username=' + encodeURIComponent(username) + '&password=' + encodeURIComponent(password);
			fetch(loginUrl, { credentials: 'include' }).then(function (res) {
				if (!res.ok) {
					return res.text().then(function (t) { throw new Error(t || 'login failed'); });
				}
				return res.json();
			}).then(function (json) {
				if (json && json.success === true) {
					var user = json.username || username;
					try { localStorage.setItem(SESSION_EXPIRES_KEY, String(Date.now() + LOGOUT_SECONDS * 1000)); } catch (e) { }
					showLoggedIn(user);
				} else {
					errorEl.textContent = json && json.message ? json.message : 'invalid username or password';
					errorEl.style.display = 'block';
				}
			}).catch(function (err) {
				errorEl.textContent = 'login failed: ' + err.message;
				errorEl.style.display = 'block';
			});
		});
	}
	window.addEventListener('load', function () {
		fetch('/session', { credentials: 'include' }).then(function (res) {
			if (!res.ok) return;
			return res.json();
		}).then(function (json) {
			if (json && json.success === true && json.username) {
				try {
					if (!localStorage.getItem(SESSION_EXPIRES_KEY)) {
						localStorage.setItem(SESSION_EXPIRES_KEY, String(Date.now() + LOGOUT_SECONDS * 1000));
					}
				} catch (e) { }
				showLoggedIn(json.username);
			}
		}).catch(function (err) { });

		window.addEventListener('storage', function (e) {
			if (e.key === SESSION_EXPIRES_KEY) {
				if (!e.newValue) {
					var resultLocal = document.getElementById('login-result');
					var containerLocal = document.getElementById('login-container');
					if (resultLocal) resultLocal.style.display = 'none';
					if (containerLocal) containerLocal.style.display = '';
				}
			}
		});
	});
})();
