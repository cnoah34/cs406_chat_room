<template>
    <div class="absolute-center">
        <button @click="router.push('/home')" class="button-gray back">Back</button>
        <div class="action-container">
            <h1 style="color: var(--vue-green);">Join a Room</h1>
            <form @submit.prevent="handleSubmit" autocomplete="off">
                <label for="room_name">Room ID</label>
                <input type="text" id="room_id" v-model="room_id"
                                                  placeholder="Enter the room ID" />

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
    
    const room_id = ref('')
    const result = ref({
        message: '',
        is_error: false,
    })

    const handleSubmit = async () => {
        result.value = { message: '', is_error: false }

        try {
            const response = await axios.patch(`${import.meta.env.VITE_REST_URL}/rooms/add-user`, 
                { room_id: room_id.value },
                {
                    headers: { 
                        Authorization: `Bearer ${localStorage.getItem('token')}`
                    }
                })

            if (response.status == 200 || response.status == 204) {
                result.value = {
                    message: 'Successfully joined room', 
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
