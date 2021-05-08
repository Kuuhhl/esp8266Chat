function changeChatText(text) {
	// split text into individual messages
	text = text.split('|')
	// load table
	let ul = document.getElementById('chat')
	// clear all messages
	ul.innerHTML = ''

	// set message if no text / hide message if there is
	let disclaimer = document.getElementById('disclaimer')
	if (text.length - 1 == 0) {
		disclaimer.style.display = ''
		disclaimer.textContent = 'Be the first one to leave a message :)'
		return
	} else {
		disclaimer.style.display = 'none'
	}
	// add messages one by one
	for (i = 0; i < text.length - 1; i++) {
		let li = document.createElement('li')
		li.appendChild(document.createTextNode(text[i]))
		ul.appendChild(li)
	}
}

// update chat every second
setInterval(function getChat() {
	fetch(`/showText`).then(function (response) {
		response.text().then(function (text) {
			changeChatText(text)
		})
	})
}, 1000)
