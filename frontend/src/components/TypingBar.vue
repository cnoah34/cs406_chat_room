<template>
    <div class="bar">
        <form @submit.prevent="sendMessage" autocomplete="off">
            <input type="text" id="content" v-model="content"
                   placeholder="Type a message..." />
            <button type="submit">Send</button>
        </form>
    </div>
</template>

<script setup>
    import { ref, onMounted } from 'vue'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'
    import axios from 'axios'

    const apiStore = useApiStore()
    const userStore = useUserStore()
    const content = ref(null)

    // Make async
    const sendMessage = async () => {
        if (content.value) {
            try {
                const response = await axios.post(
                    `${apiStore.rest_url}/messages`,
                    {
                        content: content.value,
                        room_id: userStore.current_room.room_id,
                    },
                    {
                        headers: { 
                            Authorization: `Bearer ${localStorage.getItem('token')}`
                        }
                    }
                )

                content.value = null
            }
            catch (error) {
                console.error('Error sending message: ', error)
            }
        }
    }
</script>


<style scoped>
.bar {
    display: flex;
    flex-direction: row;
    align-items: center;
    width: 100;
    min-height: 60px;
}

input {
    flex-grow: 1;
    min-height: 60px;
    padding-left: 30px;
    color: white;
    font-size: 14pt;
    background-color: var(--foreground);
    border: none;
}

input:focus {
    outline: none;
}

button {
    background-color: var(--foreground);
    border: none;
    border-left: 3px solid var(--vue-green);
    border-right: 3px solid var(--vue-green);
    color: white;
    cursor: pointer;
    font-size: 14pt;
    min-height: 60px;
}

button:hover {
    background-color: var(--vue-green);
}

</style>
