<template>
    <div class="messages-container" @scroll="handleScroll">
        <ul>
            <li v-for="message in messages" :key="message.created_at">
                <strong>{{ message.username }}</strong>: {{ message.content }}
            </li>
        </ul>
        <div v-if="loading">Loading...</div>
    </div>
</template>

<script setup>
    import { ref, onMounted } from 'vue'
    import { useRoomStore } from '@/store/room'
    import { useApiStore } from '@/store/api'
    import axios from 'axios'

    const roomStore = useRoomStore()
    const apiStore = useApiStore()

    const messages = ref([])
    const loading = ref(false)
    const hasMoreMessages = ref(true)

    const getMessages = async (before = null) => {
        try {
            loading.value = true

            const url = before
                ? `${apiStore.rest_url}/messages?room_id=${roomStore.room_id}&limit=20&before=${before}`
                : `${apiStore.rest_url}/messages?room_id=${roomStore.room_id}&limit=20`

            const response = await axios.get(url, {
                headers: { Authorization: `Bearer ${localStorage.get_item('token')}` }
            })

            if (response.data.length < 20) {
                hasMoreMessages.value = false
            }

        }
        catch (error) {
            console.error('Error fetching messages: ', error)
        }
        finally {
            loading.value = false
        }

        // Prepend older messages
        messages.value = [...response.data, ...messages.value]
    }

    const handleScroll = (event) => {
        const container = event.target

        if (container.scrollTop === 0 && hasMoreMessages.value && !loading.value) {
            const oldestMessage = messages.value[0]

            if (oldestMessage) {
                getMessages(oldestMessage.created_at)
            }
        }
    }


</script>


<style scoped>
.messages-container {
    height: 500px;
    overflow-y: auto;
    border: 1px solid var(--vue-green);
    padding: 10px;
}


</style>
