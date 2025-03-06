<template>
    <div class="container">
        <div class="messages-container" @scroll="handleScroll">
            <p v-if="!userStore.current_room" class="placeholder">Choose a room</p>

            <p v-else-if="userStore.current_room && loading.value && !messages.length"
               class="placeholder">
            There are no messages, be the first!
            </p>

            <ul>
                <li v-for="message in messages" :key="message.created_at">
                    <strong style="font-weight: bold;">{{ message.username }}</strong>: {{ message.content }}
                </li>
            </ul>
        </div>

        <TypingBar class="typing-bar"/>        
    </div>
</template>

<script setup>
    import { ref, onMounted, watch } from 'vue'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'
    import TypingBar from './TypingBar.vue'
    import axios from 'axios'

    const userStore = useUserStore()
    const apiStore = useApiStore()

    const messages = ref([])

    const loading = ref(false)
    const has_more_messages = ref(true)

    const getMessages = async (before = null) => {
        try {
            if (!userStore.current_room.room_id) {
                return
            }

            loading.value = true
            
            const limit = 20

            const url = before
            ?
            `${apiStore.rest_url}/messages/${userStore.current_room.room_id}?limit=${limit}&before=${before}`
            : `${apiStore.rest_url}/messages/${userStore.current_room.room_id}?limit=${limit}`

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

    watch(
        () => userStore.current_room,
        (new_room, old_room) => {
            if (new_room && new_room !== old_room) {
                messages.value = []
                getMessages()
            }
        },
        { immediate: true }
    )

    const handleScroll = (event) => {
        const container = event.target

        // TODO: Fix created_at being a uuid, maybe change schema
        if (container.scrollTop === 0 && has_more_messages.value && !loading.value) {
            const oldest_message = messages.value[0]
            console.log(oldest_message)

            if (oldest_message) {
                getMessages(oldest_message.created_at)
            }
        }
    }

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
    flex-direction: column;
    flex-grow: 1;
    overflow-y: auto;
    margin-top: 30px;
}

.typing-bar {
    border-top: 3px solid var(--vue-green);
    margin-top: 30px;
}

.placeholder {
    display: flex;
    align-items: center;
    justify-content: center;
    height: 80%;
    font-size: 20pt;
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
