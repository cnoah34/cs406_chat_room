import { ref } from 'vue'

class WebSocketService {
    constructor() {
        this.socket = null
        this.is_connected = false
        this.messages = ref([])
    }

    connect(room_id) {
        if (this.socket && this.is_connected) {
            this.closeConnection()
        }

        this.socket = new WebSocket(import.meta.env.VITE_WEBSOCKET_URL)

        this.socket.onopen = () => {
            //console.log('Connected to websocket server')
            try {
                const body = {
                    room_id: room_id,
                    Authorization: `Bearer ${localStorage.getItem('token')}`,
                }

                this.socket.send(JSON.stringify(body))
                this.is_connected = true
            }
            catch (error) {
                console.error('Error opening websocket connection: ', error)
            }
        }

        this.socket.onmessage = (event) => {
            const message = JSON.parse(event.data)
            this.messages.value.push(message)
        }

        
        this.socket.onclose = () => {
            //console.log('Websocket connection closed')
            this.is_connected = false
        }

        this.socket.onerror = (error) => {
            console.error('Websocket error: ', error)
        }
    }

    closeConnection() {
        if (this.socket instanceof WebSocket && this.is_connected) {
            this.socket.close()
            this.socket = null
            this.is_connected = false
        }
    }
}

export default WebSocketService
