<template>
    <div class="parent">
        <button @click="router.push('/home')" class="button-gray back">Back</button>
        <div class="container">
            <h1 style="color: var(--vue-green);">Join a Room</h1>
            <form @submit.prevent="handleSubmit" autocomplete="off">
                <label for="room_name">Room ID</label>
                <input type="text" id="room_id" v-model="room_id"
                                                  placeholder="Enter the room ID" />

                <button type="submit" class="button-green submit">Submit</button>
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
    import { useApiStore } from '@/store/api'

    const router = useRouter()
    const apiStore = useApiStore()
    
    const room_id = ref('')
    const result = ref({
        message: '',
        is_error: false,
    })

    const handleSubmit = async () => {
        result.value = { message: '', is_error: false }

        try {
            const response = await axios.patch(`${apiStore.rest_url}/rooms/add-user`, 
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
.parent {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    display: flex;
    flex-direction: column;
    height: 50vh;
    width: 25vw;
}

.back {
    font-size: 1.5rem;
    width: 6rem;
    border-bottom: none;
}

.container {
    display: flex;
    flex-direction: column;
    align-items: center;
    min-height: 340px;
    padding: 20px;
    background-color: var(--vt-c-black);
    border: 3px solid var(--vue-green);
    background-color: var(--foreground);
}

h1 {
    font-size: 30pt;
}

form {
    display: flex;
    flex-direction: column;
    width: 90%;
}

form label {
    font-size: 20pt;
    text-align: left;
    margin-top: 20px;
}

form input {
    font-size: 14pt;
    min-height: 40px;
}

form input:focus {
    outline: none;
}

.submit {
    width: 100%;
    height: 6vh;
    font-size: 18pt;
    margin-top: 20px;
    margin-bottom: 30px;
}

</style>
