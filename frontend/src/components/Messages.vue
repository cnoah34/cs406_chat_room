<template>
    <div class="messages-container" @scroll="handleScroll">
        <ul>
            <li v-for="message in messagesStore.messages" :key="message.created_at">
                <strong style="font-weight: bold;">{{ message.username }}</strong>: {{ message.content }}
            </li>
        </ul>
    </div>
</template>

<script setup>
    import { ref, onMounted, watch } from 'vue'
    import { useRoomsStore } from '@/store/rooms'
    import { useMessagesStore } from '@/store/messages'
    import { useApiStore } from '@/store/api'
    import axios from 'axios'

    const roomsStore = useRoomsStore()
    const messagesStore = useMessagesStore()
    const apiStore = useApiStore()

    const loading = ref(false)
    const hasMoreMessages = ref(true)

    const getMessages = async (before = null) => {
        try {
            if (!roomsStore.current_room.room_id) {
                console.log('No room selected')
                return
            }

            loading.value = true

            const url = before
            ? `${apiStore.rest_url}/messages/${roomsStore.current_room.room_id}?before=${before}&limit=3`
            : `${apiStore.rest_url}/messages/${roomsStore.current_room.room_id}?limit=3`

            const response = await axios.get(url, {
                headers: { Authorization: `Bearer ${localStorage.getItem('token')}` }
            })

            if (response.data.length < 20) {
                hasMoreMessages.value = false
            }

            // Prepend older messages
            if (response.data) {
                messagesStore.setMessages([...response.data, ...messagesStore.messages])
            }
        }
        catch (error) {
            console.error('Error fetching messages: ', error)
        }
        finally {
            loading.value = false
        }
    }

    watch(
        () => roomsStore.current_room,
        (new_room, old_room) => {
            if (new_room && new_room !== old_room) {
                messagesStore.clearMessages()
                getMessages()
            }
        },
        { immediate: true }
    )

    const handleScroll = (event) => {
        const container = event.target

        if (container.scrollTop === 0 && hasMoreMessages.value && !loading.value) {
            const oldestMessage = messagesStore.messages[0]

            if (oldestMessage) {
                getMessages(oldestMessage.created_at)
            }
        }
    }

    onMounted(() => {
        messagesStore.clearMessages()
    })


</script>


<style scoped>
.messages-container {
    height: 500px;
    overflow-y: auto;
    border: 1px solid var(--vue-green);
    padding: 10px;
}

ul {
    list-style-type: none;
    margin: 0;
    color: white;
    font-size: 14pt;
}

li {
    margin-top: 10px;
}


</style>
