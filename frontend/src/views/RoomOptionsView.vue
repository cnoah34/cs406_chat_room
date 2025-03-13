<template>
    <div class="parent">
        <nav>
            <button v-if="isAuthenticated" @click="handleLogout" class="logout">Logout</button>
            <router-link to="/home" class="back">Back</router-link>
        </nav>
        <div class="options-parent">
            <div class="info">
                <p>{{ userStore.current_room.name }}</p>
                <p>Room ID: {{ userStore.current_room.room_id }}</p>
            </div>
            <p style="color: white; font-size: 16pt; padding: 20px;">Privilege level: 
            <span style="color: var(--vue-green);">{{ privilege }}</span></p>
            <div class="options-container">
                <UserActions v-if="privilege === 'Owner' 
                    || privilege === 'Admin' || privilege === 'User'" />
                <AdminActions v-if="privilege === 'Owner' || privilege === 'Admin'" />
                <OwnerActions v-if="privilege === 'Owner'" />
            </div>
        </div>
    </div>
</template>


<script setup>
    import { ref, onMounted } from 'vue'
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { isAuthenticated, logout, fetchUserData } from '@/auth'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'
    import axios from 'axios'

    import OwnerActions from '@/components/OwnerActions.vue'
    import AdminActions from '@/components/AdminActions.vue'
    import UserActions from '@/components/UserActions.vue'

    const router = useRouter()
    const apiStore = useApiStore()
    const userStore = useUserStore()

    const privilege = ref('')

    onMounted( async () => {
        //Get privilege level
        try {
            const response = 
            await axios.get(`${apiStore.rest_url}/rooms/privilege/${userStore.current_room.room_id}`, {
                headers: {
                    Authorization: `Bearer ${localStorage.getItem('token')}`
                }
            })

            if (response.status == 200 && response.data && response.data.level) {
                privilege.value = response.data.level
            }
        }
        catch (error) {
            console.error('Failed to get privilege level: ', error)
        }
    })

</script>


<style scoped>
.parent {
    display: flex;
    flex-direction: column;
}

nav {
    display: flex;
    flex-direction: row-reverse;
    max-height: 40px;
}

.logout {
    width: 80px;
    height: 40px;
    font-size: 16pt;
    background-color: var(--foreground);
    border: 3px solid var(--vue-green);
    border-bottom: none;
    color: white;
    cursor: pointer;
}

.logout:hover {
    background-color: var(--vue-green);
    color: var(--foreground);
}

.back {
    display: flex;
    justify-content: center;
    align-items: center;
    width: 80px;
    height: 40px;
    font-size: 16pt;
    background-color: var(--foreground);
    border: 3px solid var(--vue-green);
    border-right: none;
    border-bottom: none;
    color: white;
    cursor: pointer;
    text-decoration: none;
}

.back:hover {
    background-color: var(--vue-green);
    color: var(--foreground);
}

.options-parent {
    display: flex;
    flex-direction: column;
    height: 85vh;
    background-color: var(--vt-c-black);
    border: 3px solid var(--vue-green);
}

.info {
    display: flex;
    flex-direction: row;
    border-bottom: 3px solid var(--vue-green);
    color: white;
    font-size: 16pt;
    padding: 10px;
    gap: 20px;
    justify-content: space-evenly;
}

.options-container {
    display: flex;
    flex-direction: row;
    justify-content: space-evenly;
}


</style>
