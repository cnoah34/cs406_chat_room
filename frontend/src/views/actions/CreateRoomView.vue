<template>
    <div class="flex-col-centered">
        <button @click="router.push('/home')" class="button-gray back">Back</button>
        <div class="action-container">
            <h1 style="color: var(--vue-green);">Create a Room</h1>
            <form @submit.prevent="handleSubmit" autocomplete="off">
                <label for="room_name">Room name</label>
                <input type="text" id="room_name" v-model="room_name"
                                                  placeholder="Enter the room name" />

                <button type="submit" class="button-gray submit">Submit</button>
            </form>
            <p v-if="result.message" :class="{'error': result.is_error, 
                     'success': !result.is_error}">{{ result.message }}</p>
        </div>
    </div>
</template>


<script setup>
    import { ref } from 'vue'
    import axios from 'axios'
    import { useRouter } from 'vue-router'

    const router = useRouter()
    
    const room_name = ref('')
    const result = ref({
        message: '',
        is_error: false,
    })

    const created_room = ref(false)

    const handleSubmit = async () => {
        if (created_room.value) {
            return;
        }

        result.value = { message: '', is_error: false }

        try {
            const response = await axios.post(`${import.meta.env.VITE_REST_URL}/rooms`, 
                { name: room_name.value },
                {
                    headers: { 
                        Authorization: `Bearer ${localStorage.getItem('token')}`
                    }
                }
            )

            if (response.status == 200 || response.status == 204) {
                created_room.value = true
                result.value = {
                    message: 'Room successfully created', 
                    is_error: false,
                }
            }
        }
        catch (error) {
            if (error.response) {
                result.value = {
                    message: error.response.data.error || 'An unexpected error occurred',
                    is_error: true,
                }
            }
            else {
                result.value = {
                    message: 'Failed to connect to the server',
                    is_error: true,
                }
            }
        }
    }
</script>


<style scoped>
@import '@/assets/actions.css';
</style>
