<template>
    <div>
        <form @submit.prevent="sendMessage" autocomplete="off" class="bar">
            <textarea id="content" v-model="content"
                                   placeholder="Type a message..."
                                   @keydown.enter.exact.prevent="sendMessage"
                                   @keydown.enter.shift.exact="lineBreak">
            </textarea>
            <button type="submit" class="button-gray submit">Send</button>
        </form>
    </div>
</template>

<script setup>
    import { ref, onMounted } from 'vue'
    import { useUserStore } from '@/store/user'
    import axios from 'axios'

    const userStore = useUserStore()
    const content = ref('')

    const lineBreak = (event) => {
        event.preventDefault();

        if (!content.value) {
            content.value = ''
        }

        content.value += '\n'
    }

    const sendMessage = async () => {
        if (content.value) {
            try {
                const response = await axios.post(
                    `${import.meta.env.VITE_REST_URL}/messages`,
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
    height: 3em;
}

textarea {
    font-family: inherit;
    font-size: 1.25rem;
    flex-grow: 1;
    padding: 10px;
    color: white;
    background-color: var(--foreground);
    border: none;
    resize: none;
}

textarea:focus {
    outline: none;
}

.submit {
    font-size: 14pt;
    min-width: 90px;
    border-top: none;
    border-bottom: none;
    border-right: none;
}

</style>
