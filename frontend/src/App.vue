<template>
    <div> 
        <nav>
            <!--<router-link v-if='!isAuthenticated' to='/'>Welcome</router-link>-->
            <router-link v-if='isAuthenticated' to='/home'>Home</router-link>
            <button v-if='isAuthenticated' @click='handleLogout'>Logout</button>
        </nav>
    </div>
    
    <RouterView />
</template>


<script setup>
    import { ref, computed, watchEffect, onMounted, onUnmounted } from 'vue'
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { isAuthenticated, logout } from './auth'

    const router = useRouter()

    const handleLogout = () => {
        logout(router)
    }

    /*
    const isAuthenticated = ref(!!localStorage.getItem('token'))

    const checkToken = () => {
        isAuthenticated.value = !!localStorage.getItem('token')
    }

    watchEffect(() => {
        checkToken()
    })

    onMounted(() => {
        window.addEventListener('storage', checkToken)
    })

    onUnmounted(() => {
        window.removeEventListener('storage', checkToken)
    })

    const logout = () => {
        localStorage.removeItem('token')
        checkToken()
        router.push('/')
    }
    */

</script>


<style>
.parent-container {
    height: 100vh;
    width: 100%;
    display: flex;
    justify-content: center;
    align-items: center;
}
</style>
