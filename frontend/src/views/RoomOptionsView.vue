<template>
    <div style="display: flex; flex-direction: column;">
        <nav style="display: flex; flex-direction: row-reverse;">
            <button @click="router.push('/home')" class="back button-gray">Back</button>
        </nav>
        <div class="options-parent">
            <div class="info">
                <p>{{ room.name }}</p>
                <p>Room ID: {{ room.room_id }}</p>
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
    import { isAuthenticated, fetchUserData } from '@/auth'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'
    import axios from 'axios'

    import OwnerActions from '@/components/OwnerActions.vue'
    import AdminActions from '@/components/AdminActions.vue'
    import UserActions from '@/components/UserActions.vue'

    const router = useRouter()
    const apiStore = useApiStore()
    const userStore = useUserStore()

    const room = userStore.current_room

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
.back {
    font-size: 1.5rem;
    width: 6rem;
    border-bottom: none;
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
    font-size: 1.25rem;
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
