import { defineStore } from 'pinia'

export const useApiStore = defineStore('api', {
    state: () => ({
        rest_url: import.meta.env.VITE_REST_URL || 'https://localhost:8080'
    }),
    persist: true,
})

export const useWebSocketStore = defineStore('websocket', {
    state: () => ({
        socket: null,
        is_connected: false,
        new_message: false,
        new_message_data: [],
    }),
    actions: {
        connectWebSocket(roomId) {
            if (this.socket && this.is_connected) {
                this.closeConnection()
            }

            const websocket_url = import.meta.env.VITE_WEBSOCKET_URL
                                || 'wss://localhost:8081/wss'

            console.log(websocket_url)

            this.socket = new WebSocket(websocket_url)
            
            this.socket.onopen = () => {
                //console.log('Connected to websocket server')

                const room_data = { room_id: roomId }
                this.socket.send(JSON.stringify(room_data))
                this.is_connected = true
            }

            this.socket.onmessage = (event) => {
                const message = JSON.parse(event.data)

                this.new_message_data = message
                this.new_message = true
            }

            this.socket.onerror = (error) => {
                console.error('Websocket error: ', error)
            }

            this.socket.onclose = () => {
                //console.log('Websocket connection closed')
                this.is_connected = false
            }
        },
        sendMessage(message) {
            if (this.socket && this.is_connected) {
                this.socket.send(JSON.stringify(message))
            }
        },
        closeConnection() {
            if (this.socket instanceof WebSocket && this.is_connected) {
                this.socket.close()
                this.is_connected = false

                localStorage.removeItem('websocket')
            }
        },
    },
    persist: true,
})
