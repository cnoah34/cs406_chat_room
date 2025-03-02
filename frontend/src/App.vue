<template>
    <div> 
        <nav>
            <router-link v-if='!isAuthenticated' to='/'>Welcome</router-link>
            <router-link v-if='isAuthenticated' to='/home'>Home</router-link>
        </nav>
    </div>
    
    <RouterView />
</template>


<script setup>
    import { ref, computed } from 'vue'
    import { RouterLink, RouterView, useRouter } from 'vue-router'

    const router = useRouter()
    const isAuthenticated = computed(() => !!localStorage.getItem('token'))

    const logout = () => {
        localStorage.removeItem('token')
        router.push('/')
    }

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
