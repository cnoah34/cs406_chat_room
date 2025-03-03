import { ref } from 'vue'
import axios from 'axios'
import { useUserStore } from '@/store/user'

async function fetchUserData() {
    try {
        const response = await axios.get('http://localhost:8080/users/metadata', {
            headers: {
                Authorization: `Bearer ${localStorage.getItem('token')}`
            }
        }) 

        const userStore = useUserStore()
        userStore.setUser(response.data)
    }
    catch (error) {
        console.error("Failed to fetch user meta data: ", error)
    }
}

export const isAuthenticated = ref(!!localStorage.getItem('token'))

export async function login(token, router) {
    localStorage.setItem('token', token)
    isAuthenticated.value = true
    await fetchUserData()
    router.push('/home')
}

export function logout(router) {
    localStorage.removeItem('token')
    isAuthenticated.value = false
    router.push('/')
}
