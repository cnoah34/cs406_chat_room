import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useMessagesStore = defineStore('messages', () => {
    const messages = ref([])

    function setMessages(data) {
        messages.value = data
    }

    function clearMessages() {
        messages.value = []
    }

    return { messages, setMessages, clearMessages }
})
