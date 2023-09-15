const chatForm = document.getElementById('chat-form');
const chatMessages = document.getElementById('chat-messages');

chatForm.addEventListener('submit', (e) => {
    e.preventDefault();
    const messagesInput = document.getElementById('message');
    const message = messagesInput.value.trim();

    if (message) {
        const messageElement = document.getElementById('div');
        messageElement.textContent = message;
        chatMessages.appendChild(messageElement);
        messageElement.value = '';
    }


});