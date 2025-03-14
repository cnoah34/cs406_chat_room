<template>
    <div class="parent">
        <router-link to="/room-options" class="back">Back</router-link>
        <div class="container">
            <h1 style="color: var(--vue-green);">Demote Admin to User</h1>
            <div class="admins-container" v-if="admins.length > 0">
                <ul>
                    <li v-for="(admin, index) in admins" :key="index" @click="selected_admin = admin"
                        :class="{ selected: admin === selected_admin }">
                        {{ admin.username }}
                    </li>
                </ul>
            </div>
            <div v-if="selected_admin">
                <p style="color: white; font-size: 14pt; padding: 10px;">Demote: {{ selected_admin.username }}</p>
            </div>
            <button @click="demoteAdmin()">Submit</button>
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

    const admins = ref([])
    
    const selected_admin = ref(null)

    const result = ref({
        message: '',
        is_error: false,
    })

    const demoteAdmin = async () => {
        result.value = { message: '', is_error: false }

        try {
            const response = await axios.patch(
            `${apiStore.rest_url}/rooms/demote-admin`, 
                { 
                    room_id: userStore.current_room.room_id,
                    user_id: selected_admin.value.user_id,
                },
                {
                    headers: {
                        Authorization: `Bearer ${localStorage.getItem('token')}`
                    }
                }
            )

            if (response.status == 200 || response.status == 204) {
                result.value = {
                    message: 'Successfully demoted admin', 
                    is_error: false,
                }
                getAdmins()
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

    const getAdmins = async () => {
        try {
            const response = 
            await axios.get(`${apiStore.rest_url}/rooms/admins/${userStore.current_room.room_id}`)

            if (response.status == 200 && response.data.admins_info) {
                admins.value = response.data.admins_info
            }
        }
        catch (error) {
            result.value = { message: 'Failed to fetch admins', is_error: true }
        }
    }

    onMounted( async () => {
        getAdmins()
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
    font-size: 24pt;
}

.admins-container {
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

button {
    width: 80px;
    height: 40px;
    font-size: 16pt;
    background-color: var(--foreground);
    border: 3px solid var(--vue-green);
    color: white;
    cursor: pointer;
    margin-top: 20px;
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
