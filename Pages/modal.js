document.addEventListener('DOMContentLoaded', function () {
	const modal = document.getElementById('modal');
	const modalClose = document.querySelector('.modal-close');
	const modalImg = document.getElementById('modal-img');
	const displayBtn = document.getElementById('displayPictureBtn');

	if (displayBtn) {
		displayBtn.addEventListener('click', function () {
			modal.style.display = 'block';
		});
	}

	if (modalClose) {
		modalClose.addEventListener('click', function () {
			modal.style.display = 'none';
		});
	}
	window.addEventListener('click', function (event) {
		if (event.target === modal) {
			modal.style.display = 'none';
		}
	});
});
