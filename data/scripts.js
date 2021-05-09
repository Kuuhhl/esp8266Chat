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

// get message file contents and call changeChatText
function getText() {
	fetch(`/showText`).then(function (response) {
		response.text().then(function (text) {
			changeChatText(text)
		})
	})
}

// initialize global cache variable.
// we need this to compare write times of the message file
var cache = ''

// get last time chat was written to and syncronize if necessary.
// executed every second
function sync() {
	setInterval(function getLastWrite() {
		fetch(`/lastWrite`).then(function (response) {
			response.text().then(function (text) {
				if (cache !== text) {
					cache = text
					getText()
				}
			})
		})
	}, 1000)
}

// start syncing on page load
window.onload = sync()
