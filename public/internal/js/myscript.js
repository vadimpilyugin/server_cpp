function showTime () {
	let time = new Date();
	let minutes = time.getMinutes() < 10 ? "0"+time.getMinutes() : time.getMinutes();
	let hours = time.getHours() < 10 ? "0"+time.getHours() : time.getHours();
	let seconds = time.getSeconds() < 10 ? "0"+time.getSeconds() : time.getSeconds();
	document.getElementById('demo').innerHTML = `${hours}:${minutes}:${seconds}`;
}