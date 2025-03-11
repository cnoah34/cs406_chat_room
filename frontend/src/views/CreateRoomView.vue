<template>
    <div class="parent">
        <h1 style="color: var(--vue-green);">Create a Room</h1>
        <form @submit.prevent="handleSubmit">
            <label for="room_name">Room name</label>
            <input type="text" id="room_name" v-model="room_name"
                                              placeholder="Enter the room name" />

            <button type="submit">Submit</button>
        </form>
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

    const handleSubmit = async () => {
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
    background-color: var(--vt-c-black);
    display: flex;
    flex-direction: column;
    height: 100%;
    width: 100%;
}

p {
    font-size: 50pt;
    color: white;
}


</style>
