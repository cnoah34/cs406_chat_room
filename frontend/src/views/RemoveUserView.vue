<template>
    <div class="parent">
        <router-link to="/home" class="back">Back</router-link>
        <div class="container">
            <h1 style="color: var(--vue-green);">Remove a User</h1>

            <!-- User list & select -->
            <div class="users-container" v-if="users.length > 0">
                <ul>
                    <li v-for="(user, index) in users" :key="index" @click="selected_user = user"
                        :class="{ selected: user === selected_user }">
                        {{ user.username }}
                    </li>
                </ul>
            </div>

            <div v-if="selected_user">
                <p style="color: white; font-size: 14pt; padding: 10px;">Remove: {{ selected_user.username }}</p>
            </div>

            <button @click="removeUser()">Submit</button>

            <p v-if="result.message" :class="{'error': result.is_error, 
                     'success': !result.is_error}">{{ result.message }}</p>
        </div>
    </div>
</template>


<script setup>
    import { ref, onMounted } from 'vue'
    import axios from 'axios'
    import { RouterLink } from 'vue-router'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'

    const apiStore = useApiStore()
    const userStore = useUserStore()

    const users = ref([])
    
    const selected_user = ref(null)

    const result = ref({
        message: '',
        is_error: false,
    })

    const removeUser = async () => {
        result.value = { message: '', is_error: false }

        try {
            const response = await axios.patch(
            `${apiStore.rest_url}/rooms/remove-user`, 
                { 
                    room_id: userStore.current_room.room_id,
                    user_id: selected_user.value.user_id,
                },
                {
                    headers: {
                        Authorization: `Bearer ${localStorage.getItem('token')}`
                    }
                }
            )

            if (response.status == 200 || response.status == 204) {
                result.value = {
                    message: 'Successfully removed user', 
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

    onMounted( async () => {
        try {
            const response = 
            await axios.get(`${apiStore.rest_url}/rooms/users/${userStore.current_room.room_id}`)

            if (response.status == 200 && response.data.users_info) {
                users.value = response.data.users_info
            }
        }
        catch (error) {
            result.value = { message: 'Failed to fetch users', is_error: true }
        }
    })
</script>


<style scoped>
.parent {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    display: flex;
    flex-direction: column;
    height: 70vh;
    width: 25vw;
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
    min-height: 340px;
    height: 70vh;
    padding: 20px;
    background-color: var(--vt-c-black);
    border: 3px solid var(--vue-green);
    background-color: var(--foreground);
}

h1 {
    font-size: 30pt;
}

.users-container {
    background-color: var(--vt-c-black);
    border: 2px solid var(--vue-green);
    height: 40vh;
    max-height: 40vh;
    padding: 10px;
}

ul {
    list-style-type: none;
    margin: 0;
    padding: 0;
    margin-bottom: 15px;
    min-width: 18vw;
    overflow-y: auto;
    flex-grow: 1;
    scrollbar-color: white var(--foreground);
}

ul::-webkit-scrollbar-thumb {
    background: white;
}

ul::-webkit-scrollbar-track {
    background: var(--foreground);
}

li {
    padding-left: 20px;
    padding-right: 20px;
    color: white;
    cursor: pointer;
    text-decoration: underline;
    font-size: 14pt;
    word-break: break-word;
}

li:hover {
    text-decoration: none;
}

.selected {
    color: var(--vue-green);
    text-decoration: none;
    cursor: default;
    font-size: 18pt;
}

.submit {
    width: 100%;
    font-size: 18pt;
    padding: 10px;
    background-color: var(--vue-green);
    border: none;
    cursor: pointer;
    margin-top: 20px;
    margin-bottom: 30px;
}

.submit:hover {
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
