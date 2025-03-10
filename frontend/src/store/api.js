import { defineStore } from 'pinia'

export const useApiStore = defineStore('api', {
    state: () => ({
        rest_url: 'http://localhost:8080',
    }),
    persist: true,
})

export const useWebSocketStore = defineStore('websocket', {
    state: () => ({
        socket: null,
        is_connected: false,
        new_message: false
    }),
    actions: {
        connectWebSocket(roomId) {
            this.socket = new WebSocket(`ws://localhost:8000/ws`)
            
            this.socket.onopen = () => {
                console.log('Connected to websocket server')

                const room_data = { room_id: roomId }
                this.socket.send(JSON.stringify(room_data))
                this.is_connected = true
            }

            this.socket.onmessage = (event) => {
                const message = event.data

                if (message === "NEW_MESSAGE") {
                    this.new_message = true
                }
            }

            this.socket.onerror = (error) => {
                console.error('Websocket error: ', error)
            }

            this.socket.onclose = () => {
                console.log('Websocket connection closed')
                this.is_connected = false
            }
        },
        sendMessage(message) {
            if (this.socket && this.is_connected) {
                this.socket.send(JSON.stringify(message))
            }
        },
        closeConnection() {
            if (this.socket) {
                this.socket.close()
                this.is_connected = false
            }
        },
    },
    persist: true,
})
