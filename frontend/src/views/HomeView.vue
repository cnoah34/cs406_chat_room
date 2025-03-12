<template>
    <div class="parent">
        <nav>
            <button v-if='isAuthenticated' @click='handleLogout'>Logout</button>
        </nav>
        <div class="home">
            <Rooms class="left" />
            <Messages class="right" />
        </div>
    </div>
</template>


<script setup>
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { isAuthenticated, logout, fetchUserData } from '../auth'
    import Rooms from '../components/Rooms.vue'
    import Messages from '../components/Messages.vue'

    const router = useRouter()

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
}

button {
    width: 80px;
    font-size: 16pt;
    background-color: var(--foreground);
    border: 3px solid var(--vue-green);
    border-bottom: none;
    color: white;
    cursor: pointer;
}

button:hover {
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
