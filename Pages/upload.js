document.addEventListener('DOMContentLoaded', function () {
	var uploadForm = document.getElementById('uploadForm');
	if (uploadForm) uploadForm.action = '/Uploads/';
});

const dropArea = document.getElementById('drop-area');
const fileInput = document.getElementById('file');
const fileList = document.getElementById('file-list');

let uploadModal = null;
function createUploadModal() {
	const modal = document.createElement('div');
	modal.className = 'modal';
	modal.style.display = 'none';
	modal.innerHTML = '<div class="modal-content"><h3>please do not close this page</h3><p>your upload is in progress. u dont want your files to be only partially uploaded, right?</p><div class="btn-spinner" aria-hidden="true"></div></div>';
	document.body.appendChild(modal);
	return modal;
}

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
	const droppedFiles = Array.from(e.dataTransfer.files);
	filesArray = filesArray.concat(droppedFiles.filter(f => !filesArray.some(existing => existing.name === f.name && existing.size === f.size)));
	updateFileInput();
	showFiles();
});
fileInput.addEventListener('change', () => {
	const selectedFiles = Array.from(fileInput.files);
	filesArray = filesArray.concat(selectedFiles.filter(f => !filesArray.some(existing => existing.name === f.name && existing.size === f.size)));
	updateFileInput();
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
	uploadModal = createUploadModal();
	let isUploading = false;
	const submitBtn = form.querySelector('button[type="submit"]') || form.querySelector('button');
	const dropAreaEl = document.getElementById('drop-area');

	function beforeUnloadHandler(e) {
		const message = 'an upload is in progress. Are you sure you want to leave?';
		e.preventDefault();
		e.returnValue = message;
		return message;
	}

	function setUploading(state) {
		isUploading = state;
		if (submitBtn) {
			submitBtn.disabled = state;
			submitBtn.dataset.origText = submitBtn.dataset.origText || submitBtn.textContent;
			submitBtn.textContent = state ? 'uploading...' : (submitBtn.dataset.origText || 'upload');
			submitBtn.classList.toggle('disabled', state);
		}
		if (fileInput) fileInput.disabled = state;
		if (dropAreaEl) dropAreaEl.classList.toggle('disabled', state);
		if (uploadModal) {
			uploadModal.style.display = state ? 'flex' : 'none';
		}
		if (state) {
			window.addEventListener('beforeunload', beforeUnloadHandler);
		} else {
			window.removeEventListener('beforeunload', beforeUnloadHandler);
		}
	}

	form.addEventListener('submit', async function (e) {
		e.preventDefault();
		if (isUploading) return;
		const fileInput = document.getElementById('file');
		const files = fileInput.files;
		if (!files.length) {
			alert('please select a file to upload.');
			return;
		}
		const formData = new FormData();
		for (let i = 0; i < files.length; i++) {
			formData.append('file', files[i]);
		}
		try {
			setUploading(true);
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
				filesArray = [];
				updateFileInput();
				showFiles();
			} else if (isJson && data && data.message) {
				alert(data.message);
			} else {
				alert('upload failed');
			}
		} catch (err) {
			alert('an error occurred while uploading.');
		} finally {
			setUploading(false);
		}
	});
});
