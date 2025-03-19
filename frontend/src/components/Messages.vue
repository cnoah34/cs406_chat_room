<template>
    <div class="container">
        <div class="messages-container" @scroll="handleScroll">
            <p v-if="!userStore.current_room" class="placeholder">Choose a room</p>
            <p v-else-if="!messages.length && !loading.value" class="placeholder">
            There are no messages, be the first!
            </p>

            <ul>
                <li v-for="message in messages" :key="message.created_at" 
                           style="list-style-type:none; margin-top: 15px;">
                    <strong class="username">{{ message.username }}</strong>
                    <span style="padding-left: 1rem;">{{ formatDate(message.created_at) }}</span>
                    <br>
                    <span v-html="formatMessage(message.content)" class="message"></span>
                </li>
            </ul>
        </div>

        <TypingBar class="typing-bar"/>        
    </div>
</template>

<script setup>
    import { ref, onMounted, watch } from 'vue'
    import axios from 'axios'
    import WebSocketService from '@/services/WebSocketService'
    import { useUserStore } from '@/store/user'
    import TypingBar from './TypingBar.vue'

    const userStore = useUserStore()
    const websocket = new WebSocketService()

    const messages = ref([])

    const loading = ref(false)
    const has_more_messages = ref(true)

    const socket = ref(null)

    const formatDate = (utc_date) => {
        if (!utc_date) return ''

        const timestamp = new Date(utc_date)

        const today = new Date()
        const is_today = timestamp.toDateString() === today.toDateString()

        const local_date = is_today
        ? ''
        : timestamp.toLocaleDateString(undefined, {
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
        })

        const local_time = timestamp.toLocaleTimeString(undefined, {
            hour: 'numeric',
            minute: '2-digit',
            hour12: true,
        })

        return `${local_date} ${local_time}`
    }

    const formatMessage = (message) => {
        return message.replace(/\n/g, '<br>')
    }

    const getOldMessages = async (before = null) => {
        try {
            if (!userStore.current_room.room_id) {
                return
            }

            loading.value = true
            
            const limit = 20

            const url = before
            ? `${import.meta.env.VITE_REST_URL}/messages/${userStore.current_room.room_id}?limit=${limit}&before=${before}`
            : `${import.meta.env.VITE_REST_URL}/messages/${userStore.current_room.room_id}?limit=${limit}`

            const response = await axios.get(url, {
                headers: { Authorization: `Bearer ${localStorage.getItem('token')}` }
            })

            if (response.data.length < limit) {
                has_more_messages.value = false
            }

            // Prepend older messages
            if (response.data) {
                messages.value = [...response.data.reverse(), ...messages.value]
            }
        }
        catch (error) {
            if (error.status != 404) {
                console.error('Error fetching messages: ', error)
            }
        }
        finally {
            loading.value = false
        }
    }

    const handleScroll = (event) => {
        const container = event.target

        const scrollbar_position = Math.abs(container.scrollTop) + container.clientHeight + 50
        const scroll_height = container.scrollHeight

        if (has_more_messages.value && !loading.value &&
            scrollbar_position >= scroll_height) {

            const oldest_message = messages.value[0]

            if (oldest_message) {
                getOldMessages(oldest_message.created_at)
            }
        }
    }

    watch(
        () => userStore.current_room,
        (new_room, old_room) => {
            if (new_room && new_room !== old_room) {
                messages.value = []
                has_more_messages.value = true
                getOldMessages()

                // Connect to the current room
                websocket.connect(userStore.current_room.room_id)
            }
        },
        { immediate: true }
    )

    // Watch for new messages received, add them to messages
    watch(
        () => websocket.messages.value,
        (new_messages) => {
            while (websocket.messages.value.length > 0) {
                messages.value.push(websocket.messages.value.shift())
            }
        },
        { deep: true, immediate: true }
    )
    
    onMounted(() => {
        messages.value = []
    })


</script>


<style scoped>
.container {
    display: flex;
    flex-direction: column;
}

.messages-container {
    display: flex;
    flex-direction: column-reverse;
    flex-grow: 1;
    margin-bottom: 20px;
    overflow-y: auto;
    scrollbar-color: white var(--foreground);
}

.messages-container::-webkit-scrollbar-thumb {
    background: white;
}

.messages-container::-webkit-scrollbar-track {
    background: var(--foreground);
}

.typing-bar {
    flex-shrink: 0;
    border-top: 3px solid var(--vue-green);
}

.placeholder {
    font-size: 20pt;
    display: flex;
    align-items: center;
    justify-content: center;
    height: 90%;
}

.username {
    font-size: 16pt;
    font-weight: bold;
    color: var(--vue-green);
}

.message {
    font-size: 14pt;
    color: white;
    word-break: break-word;
}


</style>
