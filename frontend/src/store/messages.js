import { defineStore } from 'pinia'

export const useMessagesStore = defineStore('messages', {
    state: () => ({
        messages: []
    }),
    actions: {
        setMessages(data) {
            this.messages = data
        },
        clearMessages() {
            this.$reset()
            localStorage.removeItem('messages')
        },
    },
    persist: true,
})
