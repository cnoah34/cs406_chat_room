<template>
    <div class="parent">
        <nav>
            <button v-if="isAuthenticated" @click="handleLogout" class="logout">Logout</button>
            <router-link to="/room-options" v-if="userStore.current_room" class="room-options">
                Room Options</router-link>
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

.room-options {
    display: flex;
    justify-content: center;
    align-items: center;
    width: 140px;
    height: 40px;
    font-size: 14pt;
    background-color: var(--foreground);
    border: 3px solid var(--vue-green);
    border-right: none;
    border-bottom: none;
    color: white;
    cursor: pointer;
    text-decoration: none;
}

.room-options:hover {
    background-color: var(--vue-green);
    color: var(--foreground);
}

.home {
    background-color: var(--vt-c-black);
    display: flex;
    flex-direction: row;
    height: 85vh;
}

.home .left {
    height: 100%;
    min-width: 300px;
    border: 3px solid var(--vue-green);
}

.home .right {
    flex-grow: 1;
    height: 100%;
    border: 3px solid var(--vue-green);
    border-left: none;
}

</style>
