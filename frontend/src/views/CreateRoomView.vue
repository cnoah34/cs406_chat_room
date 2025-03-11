<template>
    <div class="parent">
        <h1 style="color: var(--vue-green);">Create a Room</h1>
        <form @submit.prevent="handleSubmit" autocomplete="off">
            <label for="room_name">Room name</label>
            <input type="text" id="room_name" v-model="room_name"
                                              placeholder="Enter the room name" />

            <button type="submit">Submit</button>
        </form>
        <p v-if="result.message" :class="{'error': result.is_error, 
                 'success': !result.is_error}">{{ result.message }}</p>
    </div>
</template>


<script setup>
    import { ref } from 'vue'
    import axios from 'axios'
    import { useRouter } from 'vue-router'
    import { useApiStore } from '@/store/api'

    const router = useRouter()
    const apiStore = useApiStore()
    
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
            const response = await axios.post(`${apiStore.rest_url}/rooms`, 
                { name: room_name.value },
                {
                    headers: { 
                        Authorization: `Bearer ${localStorage.getItem('token')}`
                    }
                })

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
.parent {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    display: flex;
    flex-direction: column;
    align-self: center;
    align-items: center;
    padding: 20px;
    height: 38vh;
    width: 25vw;
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
    width: 80%;
}

form label {
    font-size: 20pt;
    text-align: left;
    margin-top: 20px;
}

form input {
    font-size: 18pt;
    min-height: 40px;
}

form input:focus {
    outline: none;
}

button {
    width: 100%;
    font-size: 18pt;
    padding: 10px;
    background-color: var(--vue-green);
    border: none;
    cursor: pointer;
    margin-top: 20px;
    margin-bottom: 30px;
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
