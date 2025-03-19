<template>
    <div class="flex-col-centered">
        <button @click="router.push('/room-options')" class="button-gray back">Back</button>
        <div class="action-container">
            <h1 style="color: var(--vue-green);">Are you sure you want to leave?</h1>
            <button @click="leaveRoom()" class="button-gray submit">Leave</button>
            <p v-if="result.message" :class="{'error': result.is_error, 
                     'success': !result.is_error}">{{ result.message }}</p>
        </div>
    </div>
</template>


<script setup>
    import { ref } from 'vue'
    import axios from 'axios'
    import { useRouter } from 'vue-router'
    import { useUserStore } from '@/store/user'

    const router = useRouter()
    const userStore = useUserStore()

    const result = ref({
        message: '',
        is_error: false,
    })

    const leaveRoom = async () => {
        result.value = { message: '', is_error: false }

        try {
            const response = await axios.patch(
            `${import.meta.env.VITE_REST_URL}/rooms/leave-room`, 
                { room_id: userStore.current_room.room_id },
                {
                    headers: {
                        Authorization: `Bearer ${localStorage.getItem('token')}`
                    }
                }
            )

            if (response.status == 204) {
                userStore.current_room = null
                router.push('/home')
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
