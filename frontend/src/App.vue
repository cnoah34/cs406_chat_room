<template>
    <div class='parent-container'> 
        <nav>
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

.content {
    flex: 1;
    width: 100%;
}

</style>
