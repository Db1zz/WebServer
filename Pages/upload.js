document.addEventListener('DOMContentLoaded', function () {
	const form = document.querySelector('form');
	if (!form) return;
	form.addEventListener('submit', async function (e) {
		e.preventDefault();
		const fileInput = document.getElementById('file');
		const files = fileInput.files;
		if (!files.length) {
			alert('Please select a file to upload.');
			return;
		}
		const formData = new FormData();
		for (let i = 0; i < files.length; i++) {
			formData.append('file', files[i]);
		}
		try {
			const response = await fetch('/Uploads/', {
				method: 'POST',
				body: formData
			});
			let data = null;
			let isJson = false;
			try {
				data = await response.json();
				isJson = true;
			} catch (jsonErr) { }
			if (response.ok && isJson && data && data.success) {
				alert(data.message);
			} else if (isJson && data && data.message) {
				alert(data.message);
			} else {
				alert('Upload failed. Server did not return a valid response.');
			}
		} catch (err) {
			alert('An error occurred while uploading.');
		}
	});
});
