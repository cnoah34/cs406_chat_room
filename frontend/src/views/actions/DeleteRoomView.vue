<template>
    <div class="parent">
        <router-link to="/room-options" class="back">Back</router-link>
        <div class="container">
            <h1 style="color: var(--vue-green);">Are you sure you want to delete the room?</h1>
            <button @click="deleteRoom()">Delete</button>
            <p v-if="result.message" :class="{'error': result.is_error, 
                     'success': !result.is_error}">{{ result.message }}</p>
        </div>
    </div>
</template>


<script setup>
    import { ref } from 'vue'
    import axios from 'axios'
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'

    const router = useRouter()
    const apiStore = useApiStore()
    const userStore = useUserStore()

    const result = ref({
        message: '',
        is_error: false,
    })

    const deleteRoom = async () => {
        result.value = { message: '', is_error: false }

        try {
            const response = await axios.delete(
            `${apiStore.rest_url}/rooms/${userStore.current_room.room_id}`, 
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
.parent {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    display: flex;
    flex-direction: column;
}

.back {
    display: flex;
    justify-content: center;
    align-items: center;
    width: 80px;
    height: 40px;
    font-size: 14pt;
    background-color: var(--foreground);
    border: 3px solid var(--vue-green);
    border-bottom: none;
    color: white;
    cursor: pointer;
    text-decoration: none;
}

.back:hover {
    background-color: var(--vue-green);
    color: var(--foreground);
}

.container {
    display: flex;
    flex-direction: column;
    align-items: center;
    height: 250px;
    padding: 20px;
    background-color: var(--vt-c-black);
    border: 3px solid var(--vue-green);
    background-color: var(--foreground);
}

h1 {
    font-size: 18pt;
    margin-top: 20px;
}

button {
    width: 100px;
    min-height: 50px;
    font-size: 16pt;
    background-color: var(--foreground);
    border: 3px solid var(--vue-green);
    color: white;
    cursor: pointer;
    margin-top: 40px;
    margin-bottom: 20px;
}

button:hover {
    background-color: var(--vue-green);
}

.error {
    color: red;
    font: bold;
}

.success {
    color: var(--vue-green);
    font: bold;
}

</style>
