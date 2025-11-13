let numFiles = 0;

function createFileElement(filename) {
	const filesList = document.getElementById("files_list");
	const fileContainer = document.createElement("div");
	fileContainer.className = "file-item";

	const nameSpan = document.createElement("span");
	nameSpan.textContent = filename;
	nameSpan.className = "file-name";

	const buttonGroup = document.createElement("div");
	buttonGroup.className = "file-button-group";

	const downloadButton = document.createElement("button");
	downloadButton.textContent = "Download";
	downloadButton.className = "file-download-button";
	downloadButton.onclick = async () => {
		if (downloadButton.disabled) return;
		downloadButton.disabled = true;
		const origText = downloadButton.textContent;
		downloadButton.textContent = 'Downloading...';
		try {
			const resp = await fetch(`/Uploads/${filename}`);
			if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
			const blob = await resp.blob();
			const url = URL.createObjectURL(blob);
			const link = document.createElement('a');
			link.href = url;
			link.download = filename;
			document.body.appendChild(link);
			link.click();
			document.body.removeChild(link);
			setTimeout(() => URL.revokeObjectURL(url), 10000);
		} catch (err) {
			console.error('download error', err);
			alert('Failed to download file.');
		} finally {
			downloadButton.disabled = false;
			downloadButton.textContent = origText;
		}
	};

	const deleteButton = document.createElement("button");
	deleteButton.textContent = "Delete";
	deleteButton.className = "file-delete-button";
	deleteButton.onclick = () => deleteFile(filename);

	buttonGroup.appendChild(downloadButton);
	buttonGroup.appendChild(deleteButton);

	fileContainer.appendChild(nameSpan);
	fileContainer.appendChild(buttonGroup);
	filesList.appendChild(fileContainer);
}

function loadFiles() {
	const filesList = document.getElementById("files_list");
	filesList.innerHTML = '';
	fetch('/Uploads/')
		.then(response => {
			if (!response.ok)
				throw new Error(`HTTP error! status: ${response.status}`);
			return response.json();
		})
		.then(files => {
			files.forEach(filename => {
				createFileElement(filename);
			});
		});
}

function deleteFile(filename) {
	if (!confirm(`are you suure you want to delete ${filename}?`))
		return;
	fetch(`/Uploads/${filename}`, {
		method: 'DELETE'
	})
		.then(response => {
			if (response.ok) {
				alert(`${filename} is gone...`);
				loadFiles();
			} else
				alert(`failed to delete ${filename}. status: ${response.status}`);
		})
		.catch(error => {
			console.error('error:', error);
			alert('ERROR!');
		});
}

document.addEventListener('DOMContentLoaded', loadFiles);