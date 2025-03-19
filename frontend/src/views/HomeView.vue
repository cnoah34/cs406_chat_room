<template>
    <div class="wrapper">
        <nav style="display: flex; flex-direction: row-reverse;">
            <button v-if="isAuthenticated" @click="handleLogout" class="button-gray logout">Logout</button>
            <button v-if="userStore.current_room"
                    @click="router.push('room-options')" class="button-gray room-options">Room Options</button>
        </nav>
        <div class="home">
            <Rooms class="left" />
            <Messages class="right" />
        </div>
    </div>
</template>


<script setup>
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { isAuthenticated, logout, fetchUserData } from '@/auth'
    import { useUserStore } from '@/store/user'
    import Rooms from '../components/Rooms.vue'
    import Messages from '../components/Messages.vue'

    const router = useRouter()
    const userStore = useUserStore()

    const handleLogout = () => {
        logout(router)
    }
</script>


<style scoped>
.wrapper {
    display: flex;
    flex-direction: column;
    min-width: 960px;
}

.logout {
    font-size: 1.5rem;
    width: 7rem;
    border-bottom: none;
 }

.room-options {
    font-size: 1.5rem;
    width: 11rem;
    border-bottom: none;
    border-right: none;
 }

.home {
    background-color: var(--vt-c-black);
    display: flex;
    flex-direction: row;
    max-height: 80vh;
}

.home .left {
    min-width: 300px;
    border: 3px solid var(--vue-green);
}

.home .right {
    flex-grow: 1;
    border: 3px solid var(--vue-green);
    border-left: none;
}

</style>
