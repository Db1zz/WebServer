(function () {
	var USERS_FILE = 'secrets/users.txt';
	var form = document.getElementById('loginForm');
	var container = document.getElementById('login-container');
	var resultEl = document.getElementById('login-result');
	var errorEl = document.getElementById('login-error');

	var LOGOUT_SECONDS = 60; // FOR TESTING

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
		container.style.display = 'none';
		errorEl.style.display = 'none';
		resultEl.style.display = 'block';
		resultEl.innerHTML = '';

		var box = document.createElement('div');
		box.className = 'upload-container login-success';
		box.style.textAlign = 'center';
		box.innerHTML = '<h2>you are logged in as <span class="login-username">' +
			username + '</span></h2>';

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

		resultEl.appendChild(box);

		var remaining = LOGOUT_SECONDS;
		var timer;

		function doLogout() {
			if (timer) clearInterval(timer);
			resultEl.style.display = 'none';
			container.style.display = '';
			var u = document.getElementById('username'); if (u) u.value = '';
			var p = document.getElementById('password'); if (p) p.value = '';
		}

		logoutBtn.addEventListener('click', function () {
			doLogout();
		});

		timer = setInterval(function () {
			remaining -= 1;
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

			fetch(USERS_FILE).then(function (res) {
				if (!res.ok) throw new Error('failed to load users file');
				return res.text();
			}).then(function (text) {
				var users = parseUsers(text);
				if (users.hasOwnProperty(username) && users[username] === password) {
					showLoggedIn(username);
				} else {
					errorEl.textContent = 'invalid username or password';
					errorEl.style.display = 'block';
				}
			}).catch(function (err) {
				errorEl.textContent = 'login failed: ' + err.message;
				errorEl.style.display = 'block';
			});
		});
	}
})();
