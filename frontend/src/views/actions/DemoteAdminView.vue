<template>
    <div class="flex-col-centered">
        <button @click="router.push('/room-options')" class="button-gray back">Back</button>
        <div class="action-container">
            <h1 style="color: var(--vue-green);">Demote Admin to User</h1>
            <div class="users-container" v-if="admins.length > 0">
                <ul class="user-list-ul">
                    <li v-for="(admin, index) in admins" :key="index" 
                               @click="selected_admin = admin" class="user-list-li" 
                               :class="{ selected: admin === selected_admin }">
                        {{ admin.username }}
                    </li>
                </ul>
            </div>
            <div v-if="selected_admin">
                <p style="color: white; font-size: 1.5rem; padding: 10px;">Demote: {{ selected_admin.username }}</p>
            </div>
            <button @click="demoteAdmin()" class="button-gray submit">Submit</button>
            <p v-if="result.message" :class="{'error': result.is_error, 
                     'success': !result.is_error}">{{ result.message }}</p>
        </div>
    </div>
</template>


<script setup>
    import { ref, onMounted } from 'vue'
    import axios from 'axios'
    import { useRouter } from 'vue-router'
    import { useUserStore } from '@/store/user'
    
    const router = useRouter()
    const userStore = useUserStore()

    const admins = ref([])
    
    const selected_admin = ref(null)

    const result = ref({
        message: '',
        is_error: false,
    })

    const demoteAdmin = async () => {
        if (!selected_admin.value) return

        result.value = { message: '', is_error: false }

        try {
            const response = await axios.patch(
            `${import.meta.env.VITE_REST_URL}/rooms/demote-admin`, 
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
            await axios.get(`${import.meta.env.VITE_REST_URL}/rooms/admins/${userStore.current_room.room_id}`)

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
@import '@/assets/actions.css';
</style>
