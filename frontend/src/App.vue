<template>
    <div class='parent-container'> 
        <nav>
            <router-link v-if='isAuthenticated' to='/home'>Home</router-link>
            <button v-if='isAuthenticated' @click='handleLogout'>Logout</button>
        </nav>
        <div class='content'>
            <RouterView />
        </div>
    </div>
    
</template>


<script setup>
    import { ref, computed, watchEffect, onMounted, onUnmounted } from 'vue'
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { isAuthenticated, logout } from './auth'

    const router = useRouter()

    const handleLogout = () => {
        logout(router)
    }
</script>


<style>
.parent-container {
    display: flex;
    flex-direction: column;
}

.content {
    flex: 1;
    width: 100%;
}

</style>
