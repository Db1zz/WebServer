document.addEventListener('DOMContentLoaded', function () {
	var uploadForm = document.getElementById('uploadForm');
	if (uploadForm) uploadForm.action = '/Uploads/';
});

const dropArea = document.getElementById('drop-area');
const fileInput = document.getElementById('file');
const fileList = document.getElementById('file-list');

let filesArray = [];

document.querySelector('.file-label').addEventListener('click', (e) => {
	e.preventDefault();
	fileInput.click();
});
dropArea.addEventListener('dragover', (e) => {
	e.preventDefault();
	dropArea.classList.add('dragover');
});
dropArea.addEventListener('dragleave', () => {
	dropArea.classList.remove('dragover');
});
dropArea.addEventListener('drop', (e) => {
	e.preventDefault();
	dropArea.classList.remove('dragover');
	filesArray = Array.from(e.dataTransfer.files);
	updateFileInput();
	showFiles();
});
fileInput.addEventListener('change', () => {
	filesArray = Array.from(fileInput.files);
	showFiles();
});

function showFiles() {
	fileList.innerHTML = '';
	filesArray.forEach((file, idx) => {
		const item = document.createElement('div');
		item.className = 'file-item';
		item.textContent = file.name;
		const delBtn = document.createElement('button');
		delBtn.textContent = 'Delete';
		delBtn.className = 'file-delete-button';
		delBtn.onclick = function () {
			filesArray.splice(idx, 1);
			updateFileInput();
			showFiles();
		};
		item.appendChild(delBtn);
		fileList.appendChild(item);
	});
}

function updateFileInput() {
	const dt = new DataTransfer();
	filesArray.forEach(file => dt.items.add(file));
	fileInput.files = dt.files;
}
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
